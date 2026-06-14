"""按新方案抽帧生成 snow 三状态精灵图：idle(sleep_2) / greet(wake_up+to_sleep) / sleep(wake_up+stay+to_sleep)"""
import cv2
import numpy as np
import os
import shutil

ROOT = os.path.dirname(os.path.abspath(__file__))
SPRITE = os.path.join(ROOT, "output", "sprite")
RESOURCES = os.path.join(ROOT, "..", "..", "resources", "sprites", "snow")
CELL = 256          # 输出帧大小，与 Muelsyse 一致，避免 QPixmap 内存超限
SRC_CELL = 512      # 源精灵图帧大小
COLS = 20
STRIDE = 2
TRIM_HEAD = 0
TRIM_TAIL = 0

tasks = {
    "idle": {
        "parts": [("sleep_2", "240frames.png")],
        "labels": None,
    },
    "greet": {
        "parts": [("wake up", "240frames.png"), ("to_sleep", "240frames.png")],
        "labels": None,
    },
    "sleep": {
        "parts": [("wake up", "240frames.png"), ("stay", "240frames.png"), ("to_sleep", "240frames.png")],
        "labels": ["falling", "looping", "waking"],
    },
}


def extract_frames(sheet):
    """从源精灵图逐帧提取 (list of SRC_CELL×SRC_CELL RGBA)"""
    h, w = sheet.shape[:2]
    cols = w // SRC_CELL
    rows = h // SRC_CELL
    frames = []
    for r in range(rows):
        for c in range(cols):
            frames.append(sheet[r * SRC_CELL:(r + 1) * SRC_CELL, c * SRC_CELL:(c + 1) * SRC_CELL])
    return frames


def pack_frames(frames):
    """紧凑排成精灵图，每行 COLS 列，帧大小 CELL×CELL"""
    if not frames:
        return np.zeros((CELL, CELL, 4), dtype=np.uint8)
    rows = (len(frames) + COLS - 1) // COLS
    sheet = np.zeros((rows * CELL, COLS * CELL, 4), dtype=np.uint8)
    for i, f in enumerate(frames):
        r, c = i // COLS, i % COLS
        sheet[r * CELL:(r + 1) * CELL, c * CELL:(c + 1) * CELL] = f
    return sheet


for state, cfg in tasks.items():
    print(f"\n{'=' * 50}")
    print(f"生成 {state}...")

    all_frames = []
    segments = {}
    total = 0

    for idx, (proj, fname) in enumerate(cfg["parts"]):
        path = os.path.join(SPRITE, proj, fname)
        sheet = cv2.imread(path, cv2.IMREAD_UNCHANGED)
        if sheet is None:
            raise SystemExit(f"缺少文件: {path}")

        frames = extract_frames(sheet)         # 512×512 帧
        kept = frames[::STRIDE]                 # 抽帧

        ts = TRIM_HEAD if idx > 0 else 0
        te = TRIM_TAIL if idx < len(cfg["parts"]) - 1 else 0
        if te > 0:
            kept = kept[ts:len(kept) - te]
        else:
            kept = kept[ts:]

        # 缩放至 CELL×CELL
        kept_resized = [cv2.resize(f, (CELL, CELL), interpolation=cv2.INTER_AREA) for f in kept]

        n = len(kept_resized)
        print(f"  {proj}: {len(frames)}帧 → 每{STRIDE}取1 → {n}帧 (512→{CELL}px)" +
              (f" (削头{ts} 削尾{te})" if ts or te else ""))

        if cfg["labels"]:
            label = cfg["labels"][idx]
            segments[label] = {"start": total, "end": total + n - 1}

        total += n
        all_frames.extend(kept_resized)

    result = pack_frames(all_frames)
    rh, rw = result.shape[:2]
    rows = rh // CELL
    cols = rw // CELL

    out = os.path.join(SPRITE, f"{state}_snow.png")
    cv2.imwrite(out, result)
    size_mb = os.path.getsize(out) / 1024 / 1024
    mem_mb = rw * rh * 4 / 1024 / 1024
    print(f"  精灵图: {rw}×{rh}  {rows}行×{cols}列  {len(all_frames)}帧  disk={size_mb:.1f}MB  mem≈{mem_mb:.0f}MB")

    os.makedirs(RESOURCES, exist_ok=True)
    dst = os.path.join(RESOURCES, f"{state}.png")
    shutil.copy2(out, dst)
    print(f"  已复制到: {dst}")

    if segments:
        print(f"  pet.json: \"{state}\": {{ \"rows\": {rows}, \"cols\": {cols},")
        print(f"    \"segments\": {{")
        for label, seg in segments.items():
            print(f'      "{label}": {{ "start": {seg["start"]}, "end": {seg["end"]} }},')
        print(f"    }}}}")
    else:
        print(f"  pet.json: \"{state}\": {{ \"rows\": {rows}, \"cols\": {cols} }}")
