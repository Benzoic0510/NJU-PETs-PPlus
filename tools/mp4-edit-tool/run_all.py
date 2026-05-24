"""一键运行全部 4 步"""

import subprocess
import sys
import config

STEPS = [
    ("01_extract_frames.py",   "抽帧"),
    ("02_pixelate.py",         "像素化"),
    ("03_remove_bg.py",        "去背景"),
    ("04_make_spritesheet.py", "拼精灵图"),
]


def run(script, desc):
    print(f"\n{'=' * 50}")
    print(f">> STEP: {desc} ({script})")
    print(f"{'=' * 50}")
    result = subprocess.run([sys.executable, script])
    if result.returncode != 0:
        print(f"\n[ERROR] {script} 执行失败，已中止")
        sys.exit(1)


if __name__ == "__main__":
    print(f"项目: {config.PROJECT}")
    print(f"视频: {config.VIDEO_PATH}")
    for script, desc in STEPS:
        run(script, desc)
    print(f"\n全部完成！项目名: {config.PROJECT}")
    print(f"  透明 PNG → output/transparent/{config.PROJECT}/")
    print(f"  精灵图   → output/sprite/{config.PROJECT}/sheet.png")
