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
- `data/AppConfig.h` 是例外：作为跨层配置单例，表现层和业务层均可直接引用（它不含 SQL 也不含 QPainter）

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

## 表现层开发

  主面板（MainMenu）

  ┌────────┬───────────────────────────────────────┐
  │ 标签页          │                                                  内容                                                 │
  ├────────┼───────────────────────────────────────┤
  │ 启动              │                           宠物形象选择（PetSelector）                            │
  ├────────┼───────────────────────────────────────┤
  │ 日程              │                 CalendarPanel（日历 + 列表 + 增删改）                    │
  ├────────┼───────────────────────────────────────┤
  │ 设置              │                              API Key、音量等配置                                        │
  ├────────┼───────────────────────────────────────┤
  │ 其他              │                                           留白                                                       │
  └────────┴───────────────────────────────────────┘

其中日程的页面想要实现这样的效果：
```html
<style>
*{box-sizing:border-box;margin:0;padding:0}
.wrap{display:grid;grid-template-columns:260px 1fr;gap:0;border:0.5px solid var(--color-border-tertiary);border-radius:var(--border-radius-lg);overflow:hidden;background:var(--color-background-primary);min-height:480px}
.left{border-right:0.5px solid var(--color-border-tertiary);padding:20px 16px;display:flex;flex-direction:column;gap:16px}
.cal-nav{display:flex;align-items:center;justify-content:space-between}
.cal-title{font-size:15px;font-weight:500;color:var(--color-text-primary)}
.nav-btn{background:none;border:0.5px solid var(--color-border-tertiary);border-radius:6px;width:28px;height:28px;cursor:pointer;display:flex;align-items:center;justify-content:center;color:var(--color-text-secondary);font-size:14px}
.nav-btn:hover{background:var(--color-background-secondary)}
.cal-grid{display:grid;grid-template-columns:repeat(7,1fr);gap:2px}
.dow{font-size:11px;color:var(--color-text-tertiary);text-align:center;padding:4px 0;font-weight:500}
.day{font-size:12px;text-align:center;padding:5px 2px;border-radius:6px;cursor:pointer;color:var(--color-text-secondary);position:relative;transition:background 0.15s}
.day:hover{background:var(--color-background-secondary)}
.day.today{background:#EEEDFE;color:#3C3489;font-weight:500}
.day.has-event::after{content:'';position:absolute;bottom:2px;left:50%;transform:translateX(-50%);width:4px;height:4px;border-radius:50%;background:#7F77DD}
.day.today.has-event::after{background:#3C3489}
.day.other-month{color:var(--color-text-tertiary);opacity:0.5}
.day.selected{background:#534AB7;color:#fff}
.day.selected.has-event::after{background:#fff}
.mini-upcoming{border-top:0.5px solid var(--color-border-tertiary);padding-top:14px}
.mini-label{font-size:11px;color:var(--color-text-tertiary);font-weight:500;margin-bottom:8px;letter-spacing:0.04em;text-transform:uppercase}
.mini-item{display:flex;align-items:center;gap:8px;padding:5px 0;cursor:pointer}
.mini-dot{width:6px;height:6px;border-radius:50%;flex-shrink:0}
.mini-text{font-size:12px;color:var(--color-text-secondary);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.mini-time{font-size:11px;color:var(--color-text-tertiary);margin-left:auto;flex-shrink:0}
.right{display:flex;flex-direction:column}
.right-header{display:flex;align-items:center;justify-content:space-between;padding:16px 20px 12px;border-bottom:0.5px solid var(--color-border-tertiary)}
.date-label{font-size:15px;font-weight:500;color:var(--color-text-primary)}
.view-tabs{display:flex;gap:4px}
.vtab{font-size:12px;padding:4px 10px;border-radius:6px;border:0.5px solid var(--color-border-tertiary);cursor:pointer;color:var(--color-text-secondary);background:none}
.vtab.active{background:#EEEDFE;color:#3C3489;border-color:#AFA9EC}
.timeline{flex:1;padding:12px 20px;overflow:auto;display:flex;flex-direction:column;gap:2px}
.time-row{display:grid;grid-template-columns:44px 1fr;gap:8px;align-items:start;min-height:52px}
.time-label{font-size:11px;color:var(--color-text-tertiary);padding-top:2px;text-align:right}
.time-slot{border-top:0.5px solid var(--color-border-tertiary);padding:4px 0;flex:1;min-height:52px;position:relative}
.event-card{border-radius:8px;padding:7px 10px;margin-bottom:4px;cursor:pointer;transition:opacity 0.15s}
.event-card:hover{opacity:0.85}
.event-title{font-size:13px;font-weight:500}
.event-meta{font-size:11px;margin-top:2px}
.ev-purple{background:#EEEDFE;border-left:3px solid #534AB7;border-top-left-radius:0;border-bottom-left-radius:0}
.ev-purple .event-title{color:#3C3489}
.ev-purple .event-meta{color:#534AB7}
.ev-teal{background:#E1F5EE;border-left:3px solid #1D9E75;border-top-left-radius:0;border-bottom-left-radius:0}
.ev-teal .event-title{color:#085041}
.ev-teal .event-meta{color:#0F6E56}
.ev-coral{background:#FAECE7;border-left:3px solid #D85A30;border-top-left-radius:0;border-bottom-left-radius:0}
.ev-coral .event-title{color:#712B13}
.ev-coral .event-meta{color:#993C1D}
.ev-amber{background:#FAEEDA;border-left:3px solid #BA7517;border-top-left-radius:0;border-bottom-left-radius:0}
.ev-amber .event-title{color:#633806}
.ev-amber .event-meta{color:#854F0B}
.add-btn{display:flex;align-items:center;gap:6px;font-size:13px;padding:6px 14px;border-radius:8px;border:0.5px solid var(--color-border-secondary);background:none;cursor:pointer;color:var(--color-text-secondary)}
.add-btn:hover{background:var(--color-background-secondary)}
</style>

<div class="wrap">
  <div class="left">
    <div class="cal-nav">
      <button class="nav-btn" onclick="changeMonth(-1)">&#8249;</button>
      <span class="cal-title" id="cal-title"></span>
      <button class="nav-btn" onclick="changeMonth(1)">&#8250;</button>
    </div>
    <div>
      <div class="cal-grid" id="cal-dow"></div>
      <div class="cal-grid" id="cal-days"></div>
    </div>
    <div class="mini-upcoming">
      <div class="mini-label">即将到来</div>
      <div id="mini-list"></div>
    </div>
  </div>
  <div class="right">
    <div class="right-header">
      <span class="date-label" id="right-date"></span>
      <div style="display:flex;gap:8px;align-items:center">
        <div class="view-tabs">
          <button class="vtab active" onclick="setView('day',this)">日</button>
          <button class="vtab" onclick="setView('week',this)">周</button>
        </div>
        <button class="add-btn" onclick="sendPrompt('如何在Qt中实现点击加号按钮弹出新建日程对话框？')">
          <i class="ti ti-plus" style="font-size:15px" aria-hidden="true"></i> 新建
        </button>
      </div>
    </div>
    <div class="timeline" id="timeline"></div>
  </div>
</div>

<script>
const EVENTS = [
  {date:'2026-05-12',start:'09:00',end:'09:30',title:'晨间站会',loc:'线上',color:'ev-teal'},
  {date:'2026-05-12',start:'10:00',end:'11:30',title:'人机交互课',loc:'教学楼 B301',color:'ev-purple'},
  {date:'2026-05-12',start:'14:00',end:'15:00',title:'项目组讨论',loc:'图书馆 3F',color:'ev-purple'},
  {date:'2026-05-12',start:'16:30',end:'17:00',title:'答辩预演',loc:'实验室',color:'ev-coral'},
  {date:'2026-05-13',start:'10:00',end:'12:00',title:'算法课',loc:'教学楼 A201',color:'ev-teal'},
  {date:'2026-05-13',start:'19:00',end:'21:00',title:'社团活动',loc:'活动中心',color:'ev-amber'},
  {date:'2026-05-14',start:'14:00',end:'15:30',title:'毕设指导',loc:'导师办公室',color:'ev-coral'},
  {date:'2026-05-15',start:'09:30',end:'11:00',title:'英语课',loc:'外语楼 201',color:'ev-amber'},
  {date:'2026-05-19',start:'10:00',end:'12:00',title:'期末考试',loc:'考试中心',color:'ev-coral'},
];

const DOWS = ['日','一','二','三','四','五','六'];
let cur = new Date(2026,4,1);
let selDate = '2026-05-12';

function fmtDate(y,m,d){return`${y}-${String(m+1).padStart(2,'0')}-${String(d).padStart(2,'0')}`}

function renderCal(){
  const y=cur.getFullYear(),m=cur.getMonth();
  document.getElementById('cal-title').textContent=`${y} 年 ${m+1} 月`;
  const dow=document.getElementById('cal-dow');
  dow.innerHTML=DOWS.map(d=>`<div class="dow">${d}</div>`).join('');
  const first=new Date(y,m,1).getDay();
  const last=new Date(y,m+1,0).getDate();
  const days=document.getElementById('cal-days');
  let html='';
  for(let i=0;i<first;i++){
    const d=new Date(y,m,-(first-1-i)).getDate();
    html+=`<div class="day other-month">${d}</div>`;
  }
  const today=fmtDate(new Date().getFullYear(),new Date().getMonth(),new Date().getDate());
  for(let d=1;d<=last;d++){
    const ds=fmtDate(y,m,d);
    const hasEv=EVENTS.some(e=>e.date===ds);
    const isTod=ds==='2026-05-12';
    const isSel=ds===selDate;
    let cls='day';
    if(isSel)cls+=' selected';
    else if(isTod)cls+=' today';
    if(hasEv)cls+=' has-event';
    html+=`<div class="${cls}" onclick="selectDate('${ds}')">${d}</div>`;
  }
  const rem=(7-((first+last)%7))%7;
  for(let d=1;d<=rem;d++)html+=`<div class="day other-month">${d}</div>`;
  days.innerHTML=html;
  renderMini();
}

function renderMini(){
  const upcoming=EVENTS.filter(e=>e.date>=selDate).slice(0,4);
  const colors={'ev-purple':'#7F77DD','ev-teal':'#1D9E75','ev-coral':'#D85A30','ev-amber':'#BA7517'};
  document.getElementById('mini-list').innerHTML=upcoming.map(e=>
    `<div class="mini-item"><div class="mini-dot" style="background:${colors[e.color]}"></div>
     <span class="mini-text">${e.title}</span>
     <span class="mini-time">${e.start}</span></div>`
  ).join('');
}

function selectDate(ds){
  selDate=ds;
  const p=ds.split('-');
  cur=new Date(+p[0],+p[1]-1,1);
  renderCal();
  renderTimeline();
  const d=new Date(+p[0],+p[1]-1,+p[2]);
  const wd=['日','一','二','三','四','五','六'][d.getDay()];
  document.getElementById('right-date').textContent=`${p[0]} 年 ${+p[1]} 月 ${+p[2]} 日  周${wd}`;
}

function renderTimeline(){
  const evs=EVENTS.filter(e=>e.date===selDate);
  const hours=[8,9,10,11,12,13,14,15,16,17,18,19,20];
  let html='';
  for(const h of hours){
    const label=`${String(h).padStart(2,'0')}:00`;
    const slotEvs=evs.filter(e=>parseInt(e.start)===h);
    let cards=slotEvs.map(e=>
      `<div class="event-card ${e.color}" onclick="sendPrompt('如何在Qt中实现点击日程卡片弹出详情面板？')">
        <div class="event-title">${e.title}</div>
        <div class="event-meta">${e.start} – ${e.end} &nbsp;·&nbsp; ${e.loc}</div>
      </div>`
    ).join('');
    html+=`<div class="time-row"><div class="time-label">${label}</div><div class="time-slot">${cards}</div></div>`;
  }
  document.getElementById('timeline').innerHTML=html;
}

function changeMonth(d){cur=new Date(cur.getFullYear(),cur.getMonth()+d,1);renderCal()}
function setView(v,btn){document.querySelectorAll('.vtab').forEach(b=>b.classList.remove('active'));btn.classList.add('active')}

selectDate('2026-05-12');
</script>

```



  桌宠（PetWidget）
  - 左键点击 → 触发互动动画
  - 拖拽 → 移动窗口
  - 右键 → 弹出环形菜单，选项：
    - 日程 → 右上角弹出自然语言输入框（BubbleWidget）
    - 动作 → 弹出二级环形菜单，选择动画动作

---
  建议的开发顺序：

  1. MainMenu        主面板骨架 + 四个标签页
  2. PetWidget       透明置顶窗口 + 拖拽
  3. Animator        精灵图切帧，让宠物动起来
  4. 环形菜单         RadialMenu（新增组件）
  5. BubbleWidget    自然语言输入框
  6. CalendarPanel   日程查看 + 增删改
  7. PetSelector     形象选择
  8. 设置页           API Key 等配置

## 注意事项

- `QApplication::setQuitOnLastWindowClosed(false)`：关主菜单不退出程序
- `PetWidget` 必须设置 `Qt::Tool` flag，不在任务栏显示
- 新增 `.cpp` 文件后需要重新跑 CMake（GLOB_RECURSE 不自动感知）
- `.db` 文件是运行时产物，已在 `.gitignore` 里排除
- `Theme.h` 只在 `presentation/` 里使用，业务层和数据层不引用