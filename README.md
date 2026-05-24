# NJU-PETs++

> 本项目为项目 NJU-PET 的升级版，故命名为 NJU-PETs++  
> 原项目链接：[BenzoicAcid / EL-project-NoDebug · GitLab](https://git.nju.edu.cn/Benzoic/el-project-nodebug)  
> 本项目为南京大学本科生课程《人机交互系统》的课程项目，采用 Qt6 进行开发

## 项目结构

```txt
NJU-PETs++/
├── CMakeLists.txt
├── CLAUDE.md                        # 项目上下文
├── README.md
│
├── app/                             # 应用层
│   ├── main.cpp
│   ├── Application.h
│   └── Application.cpp
│
├── presentation/                    # 表现层
│   ├── common/
│   │   └── Theme.h                  # 全局色彩、字体 token
│   ├── mainmenu/
│   │   ├── MainMenu.h               # 主窗口，侧边导航，切换页面
│   │   └── MainMenu.cpp
│   ├── pet/
│   │   ├── PetWidget.h / .cpp       # 透明置顶窗口，拖拽，右键菜单
│   │   ├── Animator.h / .cpp        # 精灵图切帧，状态播放
│   │   ├── RadialMenu.h / .cpp      # 环形菜单，右键呼出
│   │   └── BubbleWidget.h / .cpp    # 对话气泡弹窗
│   ├── calendar/
│   │   ├── CalendarPanel.h / .cpp   # 左侧小日历 + 右侧时间轴
│   │   ├── MiniCalendar.h / .cpp    # 自绘月份格子
│   │   ├── TimelineView.h / .cpp    # 时间轴，日程卡片
│   │   └── ScheduleEditPanel.h/.cpp # 日程新增 / 编辑弹窗
│   ├── selector/
│   │   └── PetSelector.h / .cpp     # 形象选择卡片网格
│   └── settings/
│       └── SettingsPanel.h / .cpp   # API Key、音量等配置
│
├── business/                        # 业务层
│   ├── ScheduleService.h / .cpp     # 增删改查，冲突检测
│   ├── ReminderService.h / .cpp     # 定时扫描，发提醒信号
│   ├── NLPService.h / .cpp          # 自然语言 → 结构化日程
│   └── PetStateManager.h / .cpp     # 决定宠物当前播放哪个动画
│
├── data/                            # 数据层
│   ├── models/
│   │   ├── Schedule.h               # 纯数据：id / title / startTime / endTime / ...
│   │   └── PetProfile.h             # 纯数据：id / name / spritePath
│   ├── DatabaseManager.h / .cpp     # 单例，管理 SQLite 连接
│   ├── ScheduleRepository.h / .cpp  # 封装所有 SQL CRUD
│   └── AppConfig.h / .cpp           # 读写 JSON 用户配置
│
└── resources/                       # 静态资源
    ├── sprites/                     # 精灵图 {petId}_{state}.png
    │   ├── example_idle.png
    │   └── example_walk.png...
    ├── sounds/                      # 音效文件
    ├── fonts/                       # 自定义字体
    └── resources.qrc                # Qt 资源文件清单
```

## 快速开始

### 环境要求

- **Qt 6.10.2**（MinGW 64-bit），安装路径 `D:/Qt/6.10.2/mingw_64`
- **Qt 自带工具链**（用 Qt Maintenance Tool 安装时勾选）：
  - 编译器：`D:/Qt/Tools/mingw1310_64/bin/g++.exe`
  - CMake：`D:/Qt/Tools/CMake_64/bin/cmake.exe`
  - Ninja：`D:/Qt/Tools/Ninja/ninja.exe`

> ⚠️ **必须使用 Qt 自带的 mingw1310_64 工具链**。CLion 内置 MinGW（GCC 13.1.0）和 msys64 ucrt64 的 g++ 与 Qt 6.10.2 的 AutoMoc 不兼容，会导致 `moc_predefs.h` 生成失败。

### 构建（PowerShell）

```powershell
# 1. 把 Qt 自带工具链放到 PATH 最前（覆盖 msys64 / CLion 内置 MinGW）
$env:Path = "D:\Qt\Tools\mingw1310_64\bin;D:\Qt\Tools\CMake_64\bin;D:\Qt\Tools\Ninja;" + $env:Path

# 2. 验证 g++ 指向 Qt 自带工具链（必须输出 D:\Qt\Tools\mingw1310_64\bin\g++.exe）
(Get-Command g++).Source

# 3. 在项目根目录下配置并构建
mkdir cmake-build-debug
cd cmake-build-debug
cmake .. -G Ninja `
         -DCMAKE_PREFIX_PATH=D:/Qt/6.10.2/mingw_64 `
         -DCMAKE_C_COMPILER=D:/Qt/Tools/mingw1310_64/bin/gcc.exe `
         -DCMAKE_CXX_COMPILER=D:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build .
```

> 显式传 `-DCMAKE_C_COMPILER` / `-DCMAKE_CXX_COMPILER` 是为了避免 CMake 自动探测时挑到错误的 g++。如果之前用错误的编译器配置过，**必须先 `Remove-Item -Recurse -Force cmake-build-debug` 清掉缓存再重来**。

或者直接用 **CLion** 打开项目：在 `Settings → Build, Execution, Deployment → Toolchains` 里把 MinGW 指向 `D:\Qt\Tools\mingw1310_64`，CMake 选项加 `-DCMAKE_PREFIX_PATH=D:/Qt/6.10.2/mingw_64`，然后点构建。

### 运行

构建成功后，在 `cmake-build-debug` 目录下找到 `NJU-PETs++.exe`，双击运行或命令行启动：

```bash
.\cmake-build-debug\NJU-PETs++.exe
```

程序启动后：
- 宠物（小蓝鲸）常驻桌面，可拖拽移动
- 左键点击宠物触发互动动画
- 右键宠物呼出环形菜单，可添加日程或切换动作
- 通过系统托盘图标可打开主菜单，管理日程和设置

### 常见问题

- **`cmake` 命令找不到**：系统 PATH 里没有 cmake。按上面"构建"小节先 `$env:Path = "...;D:\Qt\Tools\CMake_64\bin;..." + $env:Path` 把 Qt 自带 cmake 加进 PATH。
- **AutoMoc 报错 `moc_predefs.h ... Process failed with return value 1`**：CMake 用错了编译器（CLion 内置 MinGW 或 msys64 g++）。先 `(Get-Command g++).Source` 看看 g++ 指向哪里，如果不是 `D:\Qt\Tools\mingw1310_64\bin\g++.exe`，删掉 `cmake-build-debug` 目录，按上面"构建"小节重设 PATH 并显式传 `-DCMAKE_CXX_COMPILER` 重新配置。
- **新增 `.cpp` 文件后编译报错**：CMake 使用 `GLOB_RECURSE` 收集源文件，新增文件后需重新执行 `cmake ..` 让 CMake 重新扫描。
- **`.db` 数据库文件**：运行时自动生成在 `QStandardPaths::AppDataLocation`，不在项目目录中，已在 `.gitignore` 里排除。
- **`QApplication::setQuitOnLastWindowClosed(false)`**：关闭主菜单不会退出程序，宠物仍驻留桌面。通过系统托盘退出。
