//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include "business/NLPService.h"
#include "business/PetStateManager.h"
#include "business/ReminderService.h"
#include "business/ScheduleService.h"
#include "presentation/mainmenu/MainMenu.h"
#include "presentation/pet/BubbleWidget.h"
#include "presentation/pet/PetWidget.h"

#include <QObject>
#include <QSystemTrayIcon>

class Application : public QObject {
    Q_OBJECT

public:
    static Application &instance();
    void start();

private:
    Application();
    ~Application();
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    void connectSignals();
    void setupTrayIcon();

    ScheduleService m_scheduleService;
    ReminderService m_reminderService;
    NLPService      m_nlpService;         // 桌宠气泡专用
    NLPService      m_calendarNlpService; // 日历面板专用（添加 + 修改）
    PetStateManager m_petStateManager;

    MainMenu          *m_mainMenu     = nullptr;
    PetWidget         *m_petWidget    = nullptr;
    BubbleWidget      *m_bubbleWidget = nullptr;
    QSystemTrayIcon   *m_trayIcon     = nullptr;
    QMenu             *m_trayMenu     = nullptr;
};

#endif //APPLICATION_H
