"""将 to_sleep + sleep_2 横向拼成一张精灵图，作为 Pramanix 的 sleep，不抽帧"""
import cv2
import numpy as np
import os
import shutil

ROOT = os.path.dirname(os.path.abspath(__file__))
SPRITE = os.path.join(ROOT, "output", "sprite")
RESOURCES = os.path.join(ROOT, "..", "..", "resources", "sprites", "Pramanix")

# 两段：入场 → 循环
parts = [
    ("to_sleep", "240frames.png"),
    ("sleep_2",  "240frames.png"),
]
labels = ["falling", "looping"]

imgs = []
total_frames = 0
segments = {}

for (proj, fname), label in zip(parts, labels):
    path = os.path.join(SPRITE, proj, fname)
    img = cv2.imread(path, cv2.IMREAD_UNCHANGED)
    if img is None:
        raise SystemExit(f"缺少文件: {path}")
    h, w = img.shape[:2]
    cols = w // 512
    rows = h // 512
    frames = cols * rows
    imgs.append(img)

    segments[label] = {
        "start": total_frames,
        "end": total_frames + frames - 1,
    }
    print(f"{proj}: {w}x{h}  {rows}行×{cols}列  {frames}帧  → {label} [{segments[label]['start']}-{segments[label]['end']}]")
    total_frames += frames

# 横向拼接
result = np.hstack(imgs)
rh, rw = result.shape[:2]
total_cols = rw // 512
total_rows = rh // 512

out = os.path.join(SPRITE, "sleep_Pramanix.png")
cv2.imwrite(out, result)
print(f"\n拼接完成: {rw}×{rh}  {total_rows}行×{total_cols}列  {total_rows*total_cols}帧")
print(f"输出: {out}")
print(f"大小: {os.path.getsize(out) / 1024 / 1024:.1f} MB")

# 复制到 resources
os.makedirs(RESOURCES, exist_ok=True)
dst = os.path.join(RESOURCES, "sleep.png")
shutil.copy2(out, dst)
print(f"已复制到: {dst}")

# 输出 pet.json 片段
print()
print("=" * 50)
print("pet.json sleep 段（复制到 resources/sprites/Pramanix/pet.json）:")
print("=" * 50)
print('"sleep": {')
print(f'  "rows": {total_rows}, "cols": {total_cols},')
print(f'  "segments": {{')
for label, seg in segments.items():
    print(f'    "{label}": {{ "start": {seg["start"]}, "end": {seg["end"]} }},')
print(f'  }}')
print('}')
