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

    // TODO: 实例化并显示 MainMenu、PetWidget（表现层实现后补充）
}

void Application::connectSignals() {
    // 提醒触发 → 宠物进入 interact 状态
    connect(&m_reminderService, &ReminderService::remind,
            &m_petStateManager,  &PetStateManager::onRemind);

    // NLP 解析完成 → 自动添加日程
    connect(&m_nlpService, &NLPService::parsed,
            this, [this](const Schedule &s) {
                m_nlpService.blockSignals(true);
                m_scheduleService.addSchedule(s);
                m_nlpService.blockSignals(false);
            });

    // TODO: 表现层信号槽（逐步补充）
    // PetWidget::dragStarted  → PetStateManager::onDragStarted
    // PetWidget::dragEnded    → PetStateManager::onDragEnded
    // PetWidget::interacted   → PetStateManager::onInteract
    // PetWidget::nlpRequested → NLPService::parse
    // PetStateManager::stateChanged → Animator::setState
    // ReminderService::remind → BubbleWidget::showReminder
    // ScheduleService::scheduleAdded/Updated/Removed → CalendarPanel::refresh
    // NLPService::parseFailed → BubbleWidget::showError
}
