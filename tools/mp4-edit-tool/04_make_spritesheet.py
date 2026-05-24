"""第4步：将透明 PNG 拼成精灵图（全帧 + 抽帧 两份）"""

import os
import math
import cv2
import numpy as np
import config

os.makedirs(config.SPRITE_DIR, exist_ok=True)

all_files = sorted([f for f in os.listdir(config.TRANSPARENT_DIR) if f.endswith(".png")])
if not all_files:
    raise SystemExit("没有透明 PNG，请先运行 03_remove_bg.py")

# 先统一加载所有图片，避免重复 I/O
all_imgs = []
for f in all_files:
    img = cv2.imread(os.path.join(config.TRANSPARENT_DIR, f), cv2.IMREAD_UNCHANGED)
    if img is None or img.shape[2] < 4:
        print(f"  跳过无透明通道图片: {f}")
        continue
    all_imgs.append(img)

max_h = max(img.shape[0] for img in all_imgs)
max_w = max(img.shape[1] for img in all_imgs)


def build_sheet(imgs, cols, name):
    rows = math.ceil(len(imgs) / cols)
    sheet = np.zeros((rows * max_h, cols * max_w, 4), dtype=np.uint8)
    for i, img in enumerate(imgs):
        r = i // cols
        c = i % cols
        rh, rw = img.shape[:2]
        y_off = (max_h - rh) // 2
        x_off = (max_w - rw) // 2
        sheet[r * max_h + y_off:r * max_h + y_off + rh,
              c * max_w + x_off:c * max_w + x_off + rw] = img
    out = os.path.join(config.SPRITE_DIR, f"{name}.png")
    cv2.imwrite(out, sheet)
    print(f"  {name}.png  →  {cols}x{rows}（{len(imgs)} 张）")


# 全帧
build_sheet(all_imgs, config.SPRITE_COLS, f"{len(all_imgs)}frames")

# mod4=0: 帧 0,4,8,...
stride0 = all_imgs[::4]
build_sheet(stride0, config.SPRITE_COLS, f"{len(stride0)}frames_0")

# mod4=1: 帧 1,5,9,...
stride1 = all_imgs[1::4]
build_sheet(stride1, config.SPRITE_COLS, f"{len(stride1)}frames_1")

print(f"STEP4 完成 → {config.SPRITE_DIR}")
