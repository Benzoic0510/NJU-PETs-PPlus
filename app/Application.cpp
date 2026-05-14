//
// Created by BenzoicAcid on 2026/5/12.
//

#include "app/Application.h"
#include "data/AppConfig.h"
#include "data/DatabaseManager.h"

#include <QDebug>

Application &Application::instance() {
    static Application inst;
    return inst;
}

Application::Application() {}

void Application::start() {
    AppConfig::instance().load();

    if (!DatabaseManager::instance().init()) {
        qWarning() << "数据库初始化失败";
    }

    connectSignals();
    m_reminderService.start();

    m_mainMenu = new MainMenu(&m_scheduleService);
    m_mainMenu->show();

    m_petWidget = new PetWidget;
    m_petWidget->show();

    connect(m_mainMenu, &MainMenu::petSelected, m_petWidget, &PetWidget::loadPet);

    // PetWidget ↔ PetStateManager
    connect(m_petWidget, &PetWidget::dragStarted,        &m_petStateManager, &PetStateManager::onDragStarted);
    connect(m_petWidget, &PetWidget::dragEnded,          &m_petStateManager, &PetStateManager::onDragEnded);
    connect(m_petWidget, &PetWidget::interacted,         &m_petStateManager, &PetStateManager::onInteract);
    connect(&m_petStateManager, &PetStateManager::stateChanged, m_petWidget, &PetWidget::onStateChanged);
    connect(m_petWidget, &PetWidget::animationRequested, &m_petStateManager, &PetStateManager::onManualState);

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
    connect(&m_reminderService, &ReminderService::remind,       m_bubbleWidget, &BubbleWidget::showReminder);
}

void Application::connectSignals() {
    // 提醒触发 → 宠物进入 interact 状态
    connect(&m_reminderService, &ReminderService::remind,
            &m_petStateManager,  &PetStateManager::onRemind);

    // NLP 解析完成 → 尝试添加日程，成功显示 showResponse，冲突显示 showError
    connect(&m_nlpService, &NLPService::parsed,
            this, [this](const Schedule &s) {
                m_nlpService.blockSignals(true);
                const int id = m_scheduleService.addSchedule(s);
                m_nlpService.blockSignals(false);
                if (!m_bubbleWidget) return;
                if (id < 0)
                    m_bubbleWidget->showError("该时间段与已有日程冲突，请换个时间。");
                else
                    m_bubbleWidget->showResponse(s);
            });

    // PetWidget ↔ PetStateManager（m_petWidget 在 start() 里创建后再连）
    // 其余表现层信号槽在各组件创建后逐步补充：
    // PetWidget::nlpRequested → NLPService::parse
    // PetStateManager::stateChanged → PetWidget::onStateChanged
    // ReminderService::remind → BubbleWidget::showReminder
    // ScheduleService signals → CalendarPanel::refresh
    // NLPService::parseFailed → BubbleWidget::showError
}
