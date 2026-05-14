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

class Application : public QObject {
    Q_OBJECT

public:
    static Application &instance();
    void start();

private:
    Application();
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    void connectSignals();

    ScheduleService m_scheduleService;
    ReminderService m_reminderService;
    NLPService      m_nlpService;
    PetStateManager m_petStateManager;

    MainMenu     *m_mainMenu     = nullptr;
    PetWidget    *m_petWidget    = nullptr;
    BubbleWidget *m_bubbleWidget = nullptr;
};

#endif //APPLICATION_H
