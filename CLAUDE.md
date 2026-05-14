# NJU-Pets++ 项目上下文

## 项目简介

桌宠 + 日程管理应用，用 C++ Qt6 开发。桌面宠物（南大小蓝鲸形象）常驻桌面，支持自然语言管理日程、提醒、互动。课程项目：《人机交互》。

## 技术栈

- 语言：C++17
- UI 框架：Qt6（Widgets、Sql、Multimedia）
- 构建：CMake + Ninja
- 编译器：MinGW（Windows）
- 数据库：SQLite（通过 Qt SQL 模块）
- 资源：QRC 精灵图（PNG spritesheet）

## 三层架构（严格遵守）

```
presentation/   表现层：只管显示和用户输入，不写 SQL，不含业务逻辑
business/       业务层：只管逻辑，不调用 QPainter，不写 SQL
data/           数据层：只管存取，不含 UI 和业务判断
app/            应用层：组装三层，连接信号槽，不自己实现功能
```

### include 方向规则

- `presentation/` 可以 include `business/` 和 `data/models/`
- `business/` 可以 include `data/`
- `data/` 只能 include Qt 基础库和 `data/models/`
- **禁止反向 include**（数据层绝对不能 include 表现层）

### 判断是否跨层的标准

如果一个文件同时出现 `QPainter`（UI）和 `QSqlQuery`（数据库），说明跨层了，必须拆分。

## 文件结构

```
NJU-Pets++/
├── CMakeLists.txt
├── CLAUDE.md
├── .clang-format
├── .gitignore
├── app/
│   ├── main.cpp
│   ├── Application.h
│   └── Application.cpp
├── presentation/
│   ├── common/
│   │   └── Theme.h              # 色彩/字体 token，只有表现层用
│   ├── mainmenu/
│   │   ├── MainMenu.h / .cpp
│   ├── pet/
│   │   ├── PetWidget.h / .cpp   # 透明窗口、拖拽、右键菜单
│   │   ├── Animator.h / .cpp    # 精灵图切帧
│   │   └── BubbleWidget.h / .cpp
│   ├── calendar/
│   │   ├── CalendarPanel.h / .cpp
│   │   ├── MiniCalendar.h / .cpp
│   │   └── TimelineView.h / .cpp
│   └── selector/
│       ├── PetSelector.h / .cpp
├── business/
│   ├── ScheduleService.h / .cpp
│   ├── ReminderService.h / .cpp
│   ├── NLPService.h / .cpp
│   └── PetStateManager.h / .cpp
├── data/
│   ├── models/
│   │   ├── Schedule.h           # 纯数据结构，无逻辑
│   │   └── PetProfile.h
│   ├── DatabaseManager.h / .cpp # 单例，管理 QSqlDatabase
│   ├── ScheduleRepository.h / .cpp
│   └── AppConfig.h / .cpp
└── resources/
    ├── sprites/                 # 命名规则：{petId}_{state}.png
    ├── sounds/
    ├── fonts/
    └── resources.qrc
```

## 关键类职责

| 类                   | 层           | 职责                                                      |
| -------------------- | ------------ | --------------------------------------------------------- |
| `Application`        | app          | 单例，实例化三层，连接信号槽                              |
| `MainMenu`           | presentation | 主窗口，侧边导航，切换页面                                |
| `PetWidget`          | presentation | 透明置顶窗口，拖拽，右键菜单                              |
| `Animator`           | presentation | 读取 spritesheet，按 fps 切帧                             |
| `BubbleWidget`       | presentation | 宠物头顶对话气泡                                          |
| `CalendarPanel`      | presentation | 日历 + 时间轴日程视图                                     |
| `PetSelector`        | presentation | 形象选择卡片网格                                          |
| `ScheduleService`    | business     | 增删改查日程，冲突检测                                    |
| `ReminderService`    | business     | 定时扫描，发提醒信号                                      |
| `NLPService`         | business     | 自然语言 → 结构化日程数据                                 |
| `PetStateManager`    | business     | 决定宠物播哪个动画状态                                    |
| `Schedule`           | data/models  | 纯 struct：id/title/startTime/endTime/location/remindMins |
| `PetProfile`         | data/models  | 纯 struct：id/name/spritePath                             |
| `DatabaseManager`    | data         | 单例，管理 SQLite 连接                                    |
| `ScheduleRepository` | data         | 封装所有 SQL CRUD                                         |
| `AppConfig`          | data         | 读写 JSON 用户配置                                        |

## 信号槽通信约定

```
表现层 → 业务层：通过信号（表现层不持有业务层指针）
业务层 → 数据层：直接调用（业务层持有 Repository 引用）
业务层 → 表现层：通过信号（业务层不持有界面指针）
信号槽连接：统一在 Application::start() 里完成
```

## 精灵图约定

- 格式：PNG spritesheet（横排所有帧）
- 命名：`{petId}_{state}.png`，例如 `whale_idle.png`
- 当前 petId：`whale`（南大小蓝鲸）
- 状态列表：`idle` / `walk` / `drag` / `interact` / `sleep`
- 尺寸：每帧 128×128px

## 数据库

- 引擎：SQLite，运行时自动创建，不在项目目录里
- 路径：`QStandardPaths::AppDataLocation + "/desktopPet.db"`
- 所有 SQL 只在 `ScheduleRepository` 里出现

## 构建

```bash
mkdir cmake-build-debug && cd cmake-build-debug
cmake .. -DCMAKE_PREFIX_PATH=D:/Qt/6.10.2/mingw_64
cmake --build .
```

Qt 路径：`D:/Qt/6.10.2/mingw_64`

## 开发顺序

```
1. data/models/      先定好 Schedule、PetProfile 数据结构
2. data/             DatabaseManager → ScheduleRepository → AppConfig
3. business/         ScheduleService → ReminderService → PetStateManager → NLPService
4. app/Application   组装三层，连接信号槽
5. presentation/     最后写界面
```

## 注意事项

- `QApplication::setQuitOnLastWindowClosed(false)`：关主菜单不退出程序
- `PetWidget` 必须设置 `Qt::Tool` flag，不在任务栏显示
- 新增 `.cpp` 文件后需要重新跑 CMake（GLOB_RECURSE 不自动感知）
- `.db` 文件是运行时产物，已在 `.gitignore` 里排除
- `Theme.h` 只在 `presentation/` 里使用，业务层和数据层不引用