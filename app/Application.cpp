//
// Created by BenzoicAcid on 2026/5/12.
//

#include "app/Application.h"
#include "data/AppConfig.h"
#include "data/DatabaseManager.h"

#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QPixmap>

namespace {

QIcon iconForPet(const QString &petId) {
    QPixmap source(QString(":/sprites/%1/icon.png").arg(petId));
    if (source.isNull())
        source = QPixmap(":/sprites/Muelsyse/icon.png");

    QIcon icon;
    for (int size : {16, 22, 24, 32, 48, 64, 128, 256})
        icon.addPixmap(source.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    return icon;
}

} // namespace

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
    // 互动 / 提醒 / 点击默认音效：用户未自定义时使用内置资源
    {
        auto mapping = AppConfig::instance().soundMapping();
        if (!mapping.contains("greet") || mapping.value("greet").isEmpty()) {
            mapping.insert("greet", ":/sounds/Muelsyse_greet.wav");
        }
        if (!mapping.contains("remind") || mapping.value("remind").isEmpty()) {
            mapping.insert("remind", ":/sounds/remind.mp3");
        }
        m_soundEffectService.setMapping(mapping);
    }

    if (!DatabaseManager::instance().init()) {
        qWarning() << "数据库初始化失败";
    }

    connectSignals();
    m_reminderService.start();

    const QString currentPetId = AppConfig::instance().petId();

    m_mainMenu = new MainMenu(&m_scheduleService, &m_calendarNlpService);
    updateAppIcon(currentPetId);
    m_mainMenu->show();

    m_petWidget = new PetWidget;
    m_petWidget->loadPet(currentPetId);
    m_petWidget->show();

    connect(m_mainMenu, &MainMenu::petSelected, m_petWidget, &PetWidget::loadPet);
    connect(m_mainMenu, &MainMenu::petSelected, this, &Application::updateAppIcon);
    connect(m_mainMenu, &MainMenu::petScaleChanged, m_petWidget, &PetWidget::setPetScale);
    connect(m_mainMenu, &MainMenu::petSleepThresholdChanged,
            &m_petStateManager, &PetStateManager::setSleepThresholdMins);
    connect(m_mainMenu, &MainMenu::petInteractionDisabledChanged,
            m_petWidget, &PetWidget::setInteractionDisabled);

    // 绑定音效服务（必须在 PetWidget → PetStateManager 之前连接，这样 interacted
    // 的音效检查在 onInteract 改状态之前执行，避免状态已变 greet 导致误判跳过）
    // stateChanged 触发非交互类音效（greet 由用户 interacted 单独触发，避免空闲定时器自发互动出声）
    connect(&m_petStateManager, &PetStateManager::stateChanged, this, [this](const QString &state) {
        if (state != "greet" && state != "idle")
            m_soundEffectService.play(state);
    });
    connect(m_petWidget, &PetWidget::sleepWokeUp, this, [this]() { m_soundEffectService.play("wake"); });
    connect(m_petWidget, &PetWidget::interacted,  this, [this]() {
        if (!m_petStateManager.isInteractLocked())
            m_soundEffectService.play("greet");
    });
    connect(m_petWidget, &PetWidget::dragStarted, this, [this]() { m_soundEffectService.play("drag_start"); });
    connect(m_petWidget, &PetWidget::dragEnded,   this, [this]() { m_soundEffectService.play("drag_end"); });
    connect(m_petWidget, &PetWidget::animationRequested, this, [this](const QString &state) { m_soundEffectService.play(state); });
    connect(&m_reminderService, &ReminderService::remind, this, [this]() { m_soundEffectService.play("remind"); });

    // PetWidget ↔ PetStateManager
    connect(m_petWidget, &PetWidget::dragStarted,        &m_petStateManager, &PetStateManager::onDragStarted);
    connect(m_petWidget, &PetWidget::dragEnded,          &m_petStateManager, &PetStateManager::onDragEnded);
    connect(m_petWidget, &PetWidget::interacted,         &m_petStateManager, &PetStateManager::onInteract);
    connect(&m_petStateManager, &PetStateManager::stateChanged, m_petWidget, &PetWidget::onStateChanged);
    connect(m_petWidget, &PetWidget::animationRequested, &m_petStateManager, &PetStateManager::onManualState);
    connect(m_petWidget, &PetWidget::sleepWokeUp,       &m_petStateManager, &PetStateManager::onSleepWokeUp);

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
    connect(&m_reminderService, &ReminderService::remind,
            m_bubbleWidget, &BubbleWidget::showReminder);
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
    m_trayIcon = new QSystemTrayIcon(qApp->windowIcon(), this);
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

void Application::updateAppIcon(const QString &petId) {
    const QIcon icon = iconForPet(petId);
    qApp->setWindowIcon(icon);

    if (m_mainMenu)
        m_mainMenu->setWindowIcon(icon);
    if (m_trayIcon)
        m_trayIcon->setIcon(icon);
}

void Application::connectSignals() {
    connect(&m_scheduleService, &ScheduleService::scheduleUpdated,
            &m_reminderService, &ReminderService::forgetSchedule);
    connect(&m_scheduleService, &ScheduleService::scheduleRemoved,
            &m_reminderService, &ReminderService::forgetSchedule);

    // 提醒触发 → 宠物进入 greet 状态
    connect(&m_reminderService, &ReminderService::remind,
            &m_petStateManager,  &PetStateManager::onRemind);

    // NLP 解析完成 → 尝试添加日程，成功显示 showResponse，冲突显示 showError
    connect(&m_nlpService, &NLPService::parsed,
            this, [this](const Schedule &s) {
                // 只有 BubbleWidget 正在对话时才负责添加；CalendarPanel 的编辑面板自己添加
                if (!m_bubbleWidget || !m_bubbleWidget->isInChat()) return;
                const int id = m_scheduleService.addSchedule(s);
                if (id < 0) {
                    const Schedule conflict = m_scheduleService.conflictingSchedule(s);
                    if (conflict.startTime.isValid()) {
                        m_bubbleWidget->showError(
                            QString("与「%1」冲突：%2 - %3，请换个时间。")
                                .arg(conflict.title,
                                     conflict.startTime.toString("MM月dd日 HH:mm"),
                                     conflict.endTime.toString("HH:mm")));
                    } else {
                        m_bubbleWidget->showError("该时间段与已有日程冲突，请换个时间。");
                    }
                } else {
                    m_bubbleWidget->showResponse(s);
                }
            });

    // PetWidget ↔ PetStateManager（m_petWidget 在 start() 里创建后再连）
    // 其余表现层信号槽在各组件创建后逐步补充：
    // PetWidget::nlpRequested → NLPService::parse
    // PetStateManager::stateChanged → PetWidget::onStateChanged
    // ReminderService::remind → system tray notification
    // ScheduleService signals → CalendarPanel::refresh
    // NLPService::parseFailed → BubbleWidget::showError
}
