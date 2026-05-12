# NJU-PETs++

> 本项目为项目 NJU-PET 的升级版，故命名为 NJU-PETs++
>
> 原项目链接：[BenzoicAcid / EL-project-NoDebug · GitLab](https://git.nju.edu.cn/Benzoic/el-project-nodebug)
> 
> 本项目为南京大学本科生课程《人机交互系统》的课程项目，采用 Qt6 进行开发

## 项目框架

```txt
NJU-PETs++/
├── CMakeLists.txt
├── .clang-format
├── README.md
│
├── app/                        # 应用层
│   ├── main.cpp
│   ├── Application.h
│   └── Application.cpp
│
├── presentation/               # 表现层
│   ├── common/
│   │   └── Theme.h             # 全局色彩、字体 token
│   ├── mainmenu/
│   │   ├── MainMenu.h          # 主窗口，侧边导航
│   │   └── MainMenu.cpp
│   ├── pet/
│   │   ├── PetWidget.h         # 透明窗口，拖拽，右键菜单
│   │   ├── PetWidget.cpp
│   │   ├── Animator.h          # 精灵图切帧，状态播放
│   │   ├── Animator.cpp
│   │   ├── BubbleWidget.h      # 对话气泡弹窗
│   │   └── BubbleWidget.cpp
│   ├── calendar/
│   │   ├── CalendarPanel.h     # 左侧小日历 + 右侧时间轴
│   │   ├── CalendarPanel.cpp
│   │   ├── MiniCalendar.h      # 自绘月份格
│   │   ├── MiniCalendar.cpp
│   │   ├── TimelineView.h      # 时间轴，日程卡片
│   │   └── TimelineView.cpp
│   └── selector/
│       ├── PetSelector.h       # 形象选择卡片网格
│       └── PetSelector.cpp
│
├── business/                   # 业务层
│   ├── ScheduleService.h       # 增删改查，冲突检测
│   ├── ScheduleService.cpp
│   ├── ReminderService.h       # 定时扫描，发提醒信号
│   ├── ReminderService.cpp
│   ├── NLPService.h            # 自然语言解析意图和时间
│   ├── NLPService.cpp
│   ├── PetStateManager.h       # 决定宠物当前播哪个动画
│   └── PetStateManager.cpp
│
├── data/                       # 数据层
│   ├── models/
│   │   ├── Schedule.h          # 纯数据结构：id/title/time/location/remind
│   │   └── PetProfile.h        # 纯数据结构：id/name/spritePath
│   ├── DatabaseManager.h       # 单例，管理 QSqlDatabase 连接
│   ├── DatabaseManager.cpp
│   ├── ScheduleRepository.h    # 封装所有 SQL，提供 CRUD 接口
│   ├── ScheduleRepository.cpp
│   ├── AppConfig.h             # 读写 JSON，用户偏好设置
│   └── AppConfig.cpp
│
└── resources/                  # 静态资源
├── sprites/                    # 精灵图，命名规则：{petId}_{state}.png
│   └── example_idle.png
├── sounds/                     # 音效文件
├── fonts/                      # 自定义字体
└── resources.qrc               # Qt 资源文件清单
```

