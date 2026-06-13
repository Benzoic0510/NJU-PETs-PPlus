"""抽帧 + 重拼 sleep 精灵图（按 stride 抽取单帧，紧凑重排）"""
import cv2
import numpy as np
import os

ROOT = os.path.dirname(os.path.abspath(__file__))
SPRITE = os.path.join(ROOT, "output", "sprite")
CELL = 512
COLS = 10           # 新精灵图每行列数
STRIDE = 4          # 每 N 帧取 1 帧
TRIM_END   = 4      # 每段末尾削掉几帧（抽帧后的帧数）
TRIM_START = 4      # 每段开头削掉几帧（抽帧后的帧数）

parts = [
    ("to_sleep", "240frames.png"),
    ("sleep_2",  "240frames.png"),
    ("wake up",  "240frames.png"),
]
labels = ["falling", "looping", "waking"]

def extract_frames(sheet):
    """从精灵图中提取所有单帧 (list of 512x512 RGBA)"""
    h, w = sheet.shape[:2]
    cols = w // CELL
    rows = h // CELL
    frames = []
    for r in range(rows):
        for c in range(cols):
            y1, y2 = r * CELL, (r + 1) * CELL
            x1, x2 = c * CELL, (c + 1) * CELL
            frames.append(sheet[y1:y2, x1:x2])
    return frames

def pack_frames(frames):
    """把单帧列表紧凑排成精灵图，每行 COLS 列"""
    if not frames:
        return np.zeros((CELL, CELL * COLS, 4), dtype=np.uint8)
    rows = (len(frames) + COLS - 1) // COLS
    sheet = np.zeros((rows * CELL, COLS * CELL, 4), dtype=np.uint8)
    for i, f in enumerate(frames):
        r, c = i // COLS, i % COLS
        sheet[r * CELL:(r + 1) * CELL, c * CELL:(c + 1) * CELL] = f
    return sheet

all_frames = []
segments = {}
total = 0

for i, ((proj, fname), label) in enumerate(zip(parts, labels)):
    path = os.path.join(SPRITE, proj, fname)
    sheet = cv2.imread(path, cv2.IMREAD_UNCHANGED)
    all = extract_frames(sheet)
    kept = all[::STRIDE]

    # 除第一段保留开头、最后一段保留结尾外，首尾各削 TRIM 帧
    trim_s = TRIM_START if i > 0 else 0          # 第一段不削开头
    trim_e = TRIM_END   if i < len(parts) - 1 else 0  # 最后一段不削结尾
    kept = kept[trim_s : len(kept) - trim_e if trim_e > 0 else len(kept)]

    print(f"{proj}: {len(all)}帧 → 每{STRIDE}取1 → {len(kept)}帧" +
          (f" (削头{trim_s} 削尾{trim_e})" if trim_s or trim_e else ""))

    segments[label] = {"start": total, "end": total + len(kept) - 1}
    total += len(kept)
    all_frames.extend(kept)

result = pack_frames(all_frames)
rh, rw = result.shape[:2]
rows = rh // CELL
cols = rw // CELL

out = os.path.join(SPRITE, "sleep_combined.png")
cv2.imwrite(out, result)
print(f"\n输出: {rw}×{rh}  {rows}行×{cols}列  {len(all_frames)}帧")
print(f"大小: {os.path.getsize(out) / 1024 / 1024:.1f} MB")
print(f"\n-- 复制到 pet.json --")
print(f'"sleep": {{')
print(f'  "rows": {rows}, "cols": {cols},')
print(f'  "segments": {{')
for label, seg in segments.items():
    print(f'    "{label}": {{ "start": {seg["start"]}, "end": {seg["end"]} }},')
print(f'  }}')
print(f'}}')
