"""第3步：Color Key 去背景 + 收缩选区 + 去边

对标 Photoshop 桌宠透明流程：
  1. 自动检测背景色（四角采样）
  2. 边缘连通去除（安全，不伤内部）
  3. 小孤岛清除（封闭区 + 颜色近背景 → 删除）
  4. 像素块级别收缩选区（Select → Contract）
  5. 像素块级别去边（Defringe）
"""

import os
import cv2
import numpy as np
import config

os.makedirs(config.TRANSPARENT_DIR, exist_ok=True)

files = sorted([f for f in os.listdir(config.PIXELATED_DIR) if f.endswith(".png")])
if not files:
    raise SystemExit("没有像素化帧，请先运行 02_pixelate.py")

print(f"待处理帧数: {len(files)}  |  去背景方式: {config.BG_METHOD}  |  容差: {config.TOLERANCE}")


# ============================================================
# 颜色键去背景
# ============================================================

def detect_bg_color(bgr):
    """从图片四角 5×5 区域采样，取中位数作为背景色"""
    h, w = bgr.shape[:2]
    corners = np.concatenate([
        bgr[0:5, 0:5],
        bgr[0:5, w - 5:w],
        bgr[h - 5:h, 0:5],
        bgr[h - 5:h, w - 5:w],
    ]).reshape(-1, 3)
    return np.median(corners, axis=0).astype(np.uint8)


def color_key_remove(bgra, tolerance):
    """颜色键透明：边缘连通去除 + 封闭小孤岛智能清除"""
    h, w = bgra.shape[:2]
    bg_color = detect_bg_color(bgra[:, :, :3])

    diff = np.abs(bgra[:, :, :3].astype(np.int16) - bg_color.astype(np.int16))
    bg_mask = np.max(diff, axis=2) <= tolerance

    # 第1步：边缘连通去除（安全，不伤内部）
    _, labels = cv2.connectedComponents(bg_mask.astype(np.uint8), connectivity=4)
    border_labels = set()
    border_labels.update(labels[0, :].tolist())
    border_labels.update(labels[h - 1, :].tolist())
    border_labels.update(labels[:, 0].tolist())
    border_labels.update(labels[:, w - 1].tolist())
    for label in border_labels:
        if label > 0:
            bgra[labels == label, 3] = 0

    # 第2步：小孤岛清除（封闭在角色轮廓内的背景色小块）
    alpha = bgra[:, :, 3]
    remaining = (alpha > 0).astype(np.uint8)
    _, r_labels = cv2.connectedComponents(remaining, connectivity=4)
    total = alpha.size
    for label in range(1, r_labels.max() + 1):
        region = r_labels == label
        area = region.sum()
        # 小区域 + 颜色接近背景 → 孤岛，清除
        if area < total * 0.02:  # < 2% 面积
            region_colors = bgra[region, :3]
            avg_color = region_colors.mean(axis=0)
            if np.max(np.abs(avg_color.astype(int) - bg_color.astype(int))) <= tolerance * 2:
                bgra[region, 3] = 0

    return bgra


# ============================================================
# 像素块级别操作（保持像素画方格完整）
# ============================================================

def to_blocks(bgra):
    """512×512 → pixel_size×pixel_size 像素块"""
    return cv2.resize(bgra, (config.PIXEL_SIZE, config.PIXEL_SIZE),
                      interpolation=cv2.INTER_AREA)


def from_blocks(small):
    """pixel_size×pixel_size → 512×512 最近邻放大"""
    return cv2.resize(small, (config.OUTPUT_SIZE, config.OUTPUT_SIZE),
                      interpolation=cv2.INTER_NEAREST)


def contract_outer(bgra, iterations):
    """收缩选区：削掉主体最外层像素块（清除被背景污染的边缘）"""
    for _ in range(iterations):
        small = to_blocks(bgra)
        alpha = small[:, :, 3]
        opaque = (alpha >= 128).astype(np.uint8)
        # 对不透明区域做腐蚀 → 找到最外层
        eroded = cv2.erode(opaque, np.ones((3, 3), np.uint8), iterations=1)
        outer = (opaque > 0) & (eroded == 0)
        if not outer.any():
            break
        small[outer, 3] = 0
        bgra = from_blocks(small)
    return bgra


def defringe_blocks(bgra, iterations):
    """去边：剥除透明边缘外侧 1 层像素块（保守，像素小人不要超过 2）"""
    for _ in range(iterations):
        small = to_blocks(bgra)
        alpha = small[:, :, 3]
        transparent = (alpha < 128).astype(np.uint8)
        # 膨胀透明区域 → 找到紧邻透明的非透明块
        dilated = cv2.dilate(transparent, np.ones((3, 3), np.uint8), iterations=1)
        fringe = (dilated > 0) & (alpha >= 128)
        if not fringe.any():
            break
        small[fringe, 3] = 0
        bgra = from_blocks(small)
    return bgra


# ============================================================
# white 模式（简易，兼容旧配置）
# ============================================================

def white_to_transparent(bgra, threshold):
    """去除与图片四边连通的白色/近白色区域"""
    h, w = bgra.shape[:2]
    r, g, b = bgra[:, :, 2], bgra[:, :, 1], bgra[:, :, 0]
    white_mask = (r >= threshold) & (g >= threshold) & (b >= threshold)
    _, labels = cv2.connectedComponents(white_mask.astype(np.uint8), connectivity=4)
    border_labels = set()
    border_labels.update(labels[0, :].tolist())
    border_labels.update(labels[h - 1, :].tolist())
    border_labels.update(labels[:, 0].tolist())
    border_labels.update(labels[:, w - 1].tolist())
    for label in border_labels:
        if label > 0:
            bgra[labels == label, 3] = 0
    return bgra


# ============================================================
# rembg 模式（可选）
# ============================================================

session = None
if config.BG_METHOD == "rembg":
    try:
        from rembg import new_session
        session = new_session("u2net")
        print("rembg 模型加载成功")
    except ImportError:
        raise SystemExit("请先安装 rembg: pip install rembg")


def remove_bg_rembg(bgr):
    from rembg import remove
    rgba = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGBA)
    result = remove(rgba, session=session)
    return cv2.cvtColor(np.array(result), cv2.COLOR_RGBA2BGRA)


# ============================================================
# 主循环
# ============================================================

done = 0
for filename in files:
    src = os.path.join(config.PIXELATED_DIR, filename)
    dst = os.path.join(config.TRANSPARENT_DIR, filename)

    if os.path.exists(dst):
        done += 1
        continue

    img = cv2.imread(src)
    if img is None:
        print(f"  跳过损坏文件: {filename}")
        continue

    bgra = cv2.cvtColor(img, cv2.COLOR_BGR2BGRA)

    # 第1步：去背景
    if config.BG_METHOD == "rembg":
        bgra = remove_bg_rembg(img)
    elif config.BG_METHOD == "colorkey":
        bgra = color_key_remove(bgra, config.TOLERANCE)
    else:
        bgra = white_to_transparent(bgra, getattr(config, 'WHITE_THRESHOLD', 230))

    # 第2步：收缩选区（削掉被背景污染的最外层）
    if config.CONTRACT_PX > 0:
        bgra = contract_outer(bgra, config.CONTRACT_PX)

    # 第3步：去边（像素块级别，保守 1px）
    if config.DEFRINGE_PX > 0:
        bgra = defringe_blocks(bgra, config.DEFRINGE_PX)

    cv2.imwrite(dst, bgra)
    done += 1

    if done % 50 == 0:
        print(f"\r去背景进度: {done}/{len(files)}", end="", flush=True)

print(f"\r去背景进度: {done}/{len(files)}")
print(f"STEP3 完成：{done} 帧 → {config.TRANSPARENT_DIR}")
