"""第1步：将视频逐帧导出为 PNG"""

import os
import cv2
import config

os.makedirs(config.FRAME_DIR, exist_ok=True)

cap = cv2.VideoCapture(config.VIDEO_PATH)
if not cap.isOpened():
    raise SystemExit(f"无法打开视频: {config.VIDEO_PATH}")

total = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
print(f"视频总帧数: {total}")

i = 0
while True:
    ret, frame = cap.read()
    if not ret:
        break

    out = os.path.join(config.FRAME_DIR, f"{i:05d}.png")
    cv2.imwrite(out, frame)
    i += 1

    if i % 100 == 0 or i == total:
        print(f"\r抽帧进度: {i}/{total}", end="", flush=True)

cap.release()
print(f"\nSTEP1 完成：共导出 {i} 帧 → {config.FRAME_DIR}")
