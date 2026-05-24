# mp4-edit-tool

将 MP4 视频转换为像素风透明精灵图的工具链，为 NJU-PETs-PPlus 桌宠项目生成 spritesheet。

## 处理流程

```
视频 → 抽帧 → 像素化 → 去背景 → 拼精灵图
```

| 步骤 | 脚本 | 说明 |
|------|------|------|
| 1 | `01_extract_frames.py` | 逐帧导出为 PNG |
| 2 | `02_pixelate.py` | 降采样 + 最近邻放大 + 色调分离，产生像素画效果 |
| 3 | `03_remove_bg.py` | Color Key 去背景（自动检测背景色） + 收缩选区 + 去边 |
| 4 | `04_make_spritesheet.py` | 将所有透明帧拼成一张横向精灵图，同时生成 stride=0/1 抽帧版 |

## 环境

```bash
python -m venv .venv
source .venv/Scripts/activate   # Windows
pip install -r requirements.txt
```

可选依赖（AI 去背景）：

```bash
pip install rembg
```

## 使用方法

### 1. 准备视频

将 MP4 文件放入 `input/` 目录，绿色背景效果最佳。

### 2. 修改配置

编辑 `config.py`，关键参数：

```python
VIDEO_PATH = os.path.join(ROOT, "input", "action.mp4")  # 输入视频路径

# 第2步：像素化
PIXEL_SIZE = 256       # 缩小到的像素数（越小像素块越大）
OUTPUT_SIZE = 512      # 输出边长
POSTERIZE_LEVELS = 16  # 色调分离等级（0=关闭）

# 第3步：去背景
BG_METHOD = "colorkey"  # colorkey / white / rembg
TOLERANCE = 25          # 颜色容差（类似 PS 魔棒容差）
CONTRACT_PX = 1         # 收缩选区（削掉边缘污染）
DEFRINGE_PX = 1         # 去边宽度
```

### 3. 运行

一键运行全部 4 步：

```bash
python run_all.py
```

或逐步运行：

```bash
python 01_extract_frames.py
python 02_pixelate.py
python 03_remove_bg.py
python 04_make_spritesheet.py
```

## 输出

```
output/
├── pixelated/<项目名>/      # 像素化后的帧
├── transparent/<项目名>/    # 去背景后的透明 PNG
└── sprite/<项目名>/         # 精灵图
    ├── <N>frames.png        # 全帧 spritesheet
    ├── <N>frames_0.png      # 帧 0,4,8,... 抽帧版
    └── <N>frames_1.png      # 帧 1,5,9,... 抽帧版
```

`<项目名>` 自动取自输入视频的文件名（不含扩展名）。

## 去背景原理

采用 Color Key 流程，对标 Photoshop 魔棒 + 收缩选区：

1. 四角采样检测背景色
2. 边缘连通区域标记为透明（不误删内部）
3. 小孤岛清除（封闭在角色轮廓内的背景色块）
4. 像素块级别收缩选区，削掉被背景污染的最外层
5. 像素块级别去边，消除边缘残留

默认使用纯色背景（绿色/白色），也可以用 `rembg` AI 方案处理复杂背景。
