"""第2步：将每帧转为像素画风格

处理流程：
  1. 居中裁剪为正方形
  2. 降采样到 pixel_size（失去细节 → 产生色块）
  3. 最近邻放大到 output_size → 像素画效果
  4. Posterize 色调分离 → 合并相近色，背景变纯色
"""

import os
import cv2
import numpy as np
import config

os.makedirs(config.PIXELATED_DIR, exist_ok=True)

files = sorted([f for f in os.listdir(config.FRAME_DIR) if f.endswith(".png")])
if not files:
    raise SystemExit("没有帧文件，请先运行 01_extract_frames.py")

posterize_on = config.POSTERIZE_LEVELS > 0
print(f"待处理帧数: {len(files)}  |  像素尺寸: {config.PIXEL_SIZE}  |  输出边长: {config.OUTPUT_SIZE}  |  Posterize: {config.POSTERIZE_LEVELS if posterize_on else '关'}")


def posterize(img, levels):
    """色调分离：将每通道颜色归并到 levels 个等级（类似 PS Posterize）"""
    step = 256 // levels
    return ((img // step) * step).astype(np.uint8)


done = 0
for filename in files:
    src = os.path.join(config.FRAME_DIR, filename)
    dst = os.path.join(config.PIXELATED_DIR, filename)

    if os.path.exists(dst):
        done += 1
        continue

    img = cv2.imread(src)
    if img is None:
        print(f"  跳过损坏文件: {filename}")
        continue

    h, w = img.shape[:2]
    size = min(h, w)

    # 居中裁剪为正方形
    x = (w - size) // 2
    y = (h - size) // 2
    cropped = img[y:y + size, x:x + size]

    # 降采样 → 最近邻放大
    small = cv2.resize(cropped, (config.PIXEL_SIZE, config.PIXEL_SIZE),
                       interpolation=cv2.INTER_AREA)
    pixelated = cv2.resize(small, (config.OUTPUT_SIZE, config.OUTPUT_SIZE),
                           interpolation=cv2.INTER_NEAREST)

    # 色调分离 → 背景颜色统一
    if posterize_on:
        pixelated = posterize(pixelated, config.POSTERIZE_LEVELS)

    cv2.imwrite(dst, pixelated)
    done += 1

    if done % 50 == 0:
        print(f"\r像素化进度: {done}/{len(files)}", end="", flush=True)

print(f"\r像素化进度: {done}/{len(files)}")
print(f"STEP2 完成：{done} 帧 → {config.PIXELATED_DIR}")
