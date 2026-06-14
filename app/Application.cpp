//
// Created by BenzoicAcid on 2026/5/12.
//

#include "app/Application.h"
#include "data/AppConfig.h"
#include "data/DatabaseManager.h"

#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QPixmap>

Application &Application::instance() {
    static Application inst;
    return inst;
}

Application::Application() {}

Application::~Application() {
    delete m_bubbleWidget;
    delete m_petWidget;
    delete m_mainMenu;
    delete m_trayMenu;
    m_trayIcon->setContextMenu(nullptr);
}

void Application::start() {
    AppConfig::instance().load();
    m_soundEffectService.setMapping(AppConfig::instance().soundMapping());

    if (!DatabaseManager::instance().init()) {
        qWarning() << "数据库初始化失败";
    }

    connectSignals();
    m_reminderService.start();

    m_mainMenu = new MainMenu(&m_scheduleService, &m_calendarNlpService);
    m_mainMenu->show();

    m_petWidget = new PetWidget;
    m_petWidget->loadPet(AppConfig::instance().petId());
    m_petWidget->show();

    connect(m_mainMenu, &MainMenu::petSelected, m_petWidget, &PetWidget::loadPet);
    connect(m_mainMenu, &MainMenu::petScaleChanged, m_petWidget, &PetWidget::setPetScale);
    connect(m_mainMenu, &MainMenu::petSleepThresholdChanged,
            &m_petStateManager, &PetStateManager::setSleepThresholdMins);
    connect(m_mainMenu, &MainMenu::petInteractionDisabledChanged,
            m_petWidget, &PetWidget::setInteractionDisabled);

    // PetWidget ↔ PetStateManager
    connect(m_petWidget, &PetWidget::dragStarted,        &m_petStateManager, &PetStateManager::onDragStarted);
    connect(m_petWidget, &PetWidget::dragEnded,          &m_petStateManager, &PetStateManager::onDragEnded);
    connect(m_petWidget, &PetWidget::interacted,         &m_petStateManager, &PetStateManager::onInteract);
    connect(&m_petStateManager, &PetStateManager::stateChanged, m_petWidget, &PetWidget::onStateChanged);
    connect(m_petWidget, &PetWidget::animationRequested, &m_petStateManager, &PetStateManager::onManualState);
    connect(m_petWidget, &PetWidget::sleepWokeUp,       &m_petStateManager, &PetStateManager::onSleepWokeUp);

    // 绑定音效服务
    connect(&m_petStateManager, &PetStateManager::stateChanged, &m_soundEffectService, &SoundEffectService::play);
    connect(m_petWidget, &PetWidget::sleepWokeUp, this, [this]() { m_soundEffectService.play("wake"); });
    connect(m_petWidget, &PetWidget::interacted,  this, [this]() { m_soundEffectService.play("click"); });
    connect(m_petWidget, &PetWidget::dragStarted, this, [this]() { m_soundEffectService.play("drag_start"); });
    connect(m_petWidget, &PetWidget::dragEnded,   this, [this]() { m_soundEffectService.play("drag_end"); });

    // BubbleWidget
    m_bubbleWidget = new BubbleWidget;
    m_bubbleWidget->attachTo(m_petWidget);

    connect(m_petWidget,    &PetWidget::nlpRequested,          m_bubbleWidget, &BubbleWidget::showInput);
    connect(m_petWidget,    &PetWidget::interacted,             m_bubbleWidget, &BubbleWidget::dismiss);
    connect(m_petWidget,    &PetWidget::dragStarted,            m_bubbleWidget, &BubbleWidget::dismiss);
    connect(m_petWidget,    &PetWidget::interacted,             &m_nlpService,  &NLPService::clearHistory);
    connect(m_petWidget,    &PetWidget::dragStarted,            &m_nlpService,  &NLPService::clearHistory);
    connect(m_bubbleWidget, &BubbleWidget::submitted,           &m_nlpService,  &NLPService::parse);
    connect(&m_nlpService,  &NLPService::parseFailed,           m_bubbleWidget, &BubbleWidget::showError);
    connect(&m_nlpService,  &NLPService::clarificationNeeded,   m_bubbleWidget, &BubbleWidget::showClarification);
    connect(m_petWidget, &PetWidget::showSchedulePanelRequested,
            m_mainMenu, &MainMenu::showSchedulePage);
    connect(m_petWidget, &PetWidget::upcomingScheduleRequested, this, [this]() {
        m_bubbleWidget->showUpcoming(m_scheduleService.getUpcoming());
    });
    // 环形菜单：面板 / 退出
    connect(m_petWidget, &PetWidget::showMainMenuRequested, this, [this]() {
        m_mainMenu->show();
        m_mainMenu->raise();
        m_mainMenu->activateWindow();
    });
    connect(m_petWidget, &PetWidget::quitRequested, qApp, &QApplication::quit);

    setupTrayIcon();
}

void Application::setupTrayIcon() {
    const QPixmap sheet(":/sprites/Muelsyse/idle.png");
    const QIcon icon(sheet.copy(0, 0, 128, 128).scaled(
        22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_trayIcon = new QSystemTrayIcon(icon, this);
    m_trayIcon->setToolTip("NJU-PETs++");

    m_trayMenu    = new QMenu;
    auto *showAct = m_trayMenu->addAction("显示主面板");
    m_trayMenu->addSeparator();
    auto *quitAct = m_trayMenu->addAction("退出");
    m_trayIcon->setContextMenu(m_trayMenu);

    connect(showAct, &QAction::triggered, this, [this]() {
        m_mainMenu->show();
        m_mainMenu->raise();
        m_mainMenu->activateWindow();
    });
    connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::DoubleClick ||
                    reason == QSystemTrayIcon::Trigger)
                {
                    m_mainMenu->show();
                    m_mainMenu->raise();
                    m_mainMenu->activateWindow();
                }
            });

    m_trayIcon->show();
}

void Application::connectSignals() {
    // 提醒触发 → 宠物进入 interact 状态 + 系统通知
    connect(&m_reminderService, &ReminderService::remind,
            &m_petStateManager,  &PetStateManager::onRemind);
    connect(&m_reminderService, &ReminderService::remind, this, [this](const Schedule &s) {
        if (!m_trayIcon)
            return;

        const QString title = s.isDDL ? "DDL 提醒" : "日程提醒";
        QString message = s.isDDL
            ? QString("%1\n截止：%2").arg(s.title, s.startTime.toString("MM月dd日 HH:mm"))
            : QString("%1\n时间：%2").arg(s.title, s.startTime.toString("MM月dd日 HH:mm"));
        if (!s.location.isEmpty())
            message += "\n地点：" + s.location;

        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 8000);
    });

    // NLP 解析完成 → 尝试添加日程，成功显示 showResponse，冲突显示 showError
    connect(&m_nlpService, &NLPService::parsed,
            this, [this](const Schedule &s) {
                // 只有 BubbleWidget 正在对话时才负责添加；CalendarPanel 的编辑面板自己添加
                if (!m_bubbleWidget || !m_bubbleWidget->isInChat()) return;
                const int id = m_scheduleService.addSchedule(s);
                if (id < 0)
                    m_bubbleWidget->showError("该时间段与已有日程冲突，请换个时间。");
                else
                    m_bubbleWidget->showResponse(s);
            });

    // PetWidget ↔ PetStateManager（m_petWidget 在 start() 里创建后再连）
    // 其余表现层信号槽在各组件创建后逐步补充：
    // PetWidget::nlpRequested → NLPService::parse
    // PetStateManager::stateChanged → PetWidget::onStateChanged
    // ReminderService::remind → system tray notification
    // ScheduleService signals → CalendarPanel::refresh
    // NLPService::parseFailed → BubbleWidget::showError
}