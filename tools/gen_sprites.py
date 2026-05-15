"""
Generate placeholder whale sprite sheets for NJU-PETs++.
Output: resources/sprites/whale_{state}.png  (4 frames x 128x128, horizontal)
Run from project root: python tools/gen_sprites.py
"""

import math
import os
from PIL import Image, ImageDraw

FRAME = 128
OUT_DIR = "resources/sprites"

# Color palette
BODY   = (72,  144, 228)
BODY2  = (55,  120, 200)
BELLY  = (185, 220, 255)
EW     = (255, 255, 255)
EP     = (20,  20,  40)
HOLE   = (45,  100, 185)


def _draw_whale(d: ImageDraw.ImageDraw, cx: float, cy: float, scale: float = 1.0):
    """Draw a simple cartoon whale at (cx, cy)."""
    bw = int(42 * scale)
    bh = int(24 * scale)

    # Body
    d.ellipse([cx - bw, cy - bh, cx + bw, cy + bh], fill=BODY)

    # Belly patch
    d.ellipse([cx - bw + 10, cy, cx + bw - 10, cy + bh], fill=BELLY)

    # Tail flukes (left)
    tx = cx - bw
    d.polygon([(tx, cy - 3), (tx - 18, cy - 20), (tx - 6, cy + 3)], fill=BODY2)
    d.polygon([(tx, cy + 3), (tx - 18, cy + 20), (tx - 6, cy - 3)], fill=BODY2)

    # Dorsal fin (top)
    dfx, dfy = cx + 8, cy - bh
    d.polygon([(dfx - 8, dfy), (dfx + 3, dfy - 16), (dfx + 10, dfy)], fill=BODY2)

    # Pectoral fin (bottom right)
    d.polygon(
        [(cx + 8, cy + bh - 4), (cx + 20, cy + bh + 12), (cx - 4, cy + bh)],
        fill=BODY2,
    )

    # Eye
    ex, ey = cx + bw // 2 - 6, cy - bh // 4
    d.ellipse([ex - 5, ey - 5, ex + 5, ey + 5], fill=EW)
    d.ellipse([ex - 2, ey - 2, ex + 2, ey + 2], fill=EP)

    # Blowhole
    bx, by = cx + bw // 4, cy - bh + 2
    d.ellipse([bx - 4, by - 2, bx + 4, by + 2], fill=HOLE)


def _frame(dy: float = 0.0, scale: float = 1.0) -> Image.Image:
    img = Image.new("RGBA", (FRAME, FRAME), (0, 0, 0, 0))
    _draw_whale(ImageDraw.Draw(img), 64.0, 64.0 + dy, scale)
    return img


def _rotated_frame(dy: float = 0.0, angle: float = 0.0, scale: float = 1.0) -> Image.Image:
    base = _frame(0.0, scale)
    if angle:
        base = base.rotate(angle, resample=Image.BICUBIC, expand=False)
    if dy:
        shifted = Image.new("RGBA", (FRAME, FRAME), (0, 0, 0, 0))
        shifted.paste(base, (0, int(dy)))
        return shifted
    return base


def _sheet(*frames: Image.Image) -> Image.Image:
    n = len(frames)
    s = Image.new("RGBA", (FRAME * n, FRAME), (0, 0, 0, 0))
    for i, f in enumerate(frames):
        s.paste(f, (i * FRAME, 0), mask=f)
    return s


def generate_idle() -> Image.Image:
    return _sheet(
        _frame(0), _frame(-2), _frame(-3), _frame(-2)
    )


def generate_walk() -> Image.Image:
    return _sheet(
        _rotated_frame(-1, -4), _rotated_frame(0, 0),
        _rotated_frame(1,  4), _rotated_frame(0, 0),
    )


def generate_drag() -> Image.Image:
    return _sheet(
        _rotated_frame(-2, -10), _rotated_frame(-4, -14),
        _rotated_frame(-2, -10), _rotated_frame(0,  -6),
    )


def generate_interact() -> Image.Image:
    return _sheet(
        _frame(0), _frame(-8, 1.06), _frame(-14, 1.1), _frame(-8, 1.06)
    )


def generate_sleep() -> Image.Image:
    frames = []
    for dy, sc in [(4, 0.96), (5, 0.97), (4, 0.96), (3, 0.97)]:
        img = _rotated_frame(dy, 6, sc)
        # Add "z" marks as small white ellipses
        d = ImageDraw.Draw(img)
        for j, (zx, zy, r) in enumerate([(88, 28, 4), (96, 19, 3), (103, 12, 2)]):
            alpha = max(60, 200 - j * 60)
            d.ellipse([zx - r, zy - r, zx + r, zy + r],
                      fill=(120, 170, 240))
        frames.append(img)
    return _sheet(*frames)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    specs = {
        "idle":     generate_idle,
        "walk":     generate_walk,
        "drag":     generate_drag,
        "interact": generate_interact,
        "sleep":    generate_sleep,
    }
    for state, fn in specs.items():
        path = f"{OUT_DIR}/whale_{state}.png"
        fn().save(path)
        print(f"  {path}")
    print("Done.")


if __name__ == "__main__":
    main()
