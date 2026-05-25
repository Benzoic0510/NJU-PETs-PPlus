//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef MAINMENU_H
#define MAINMENU_H

#include "business/NLPService.h"
#include "business/ScheduleService.h"

#include <QButtonGroup>
#include <QLabel>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QStackedWidget>
#include <QVariantAnimation>
#include <QWidget>

#include <functional>

class CalendarPanel;

class MainMenu : public QWidget {
    Q_OBJECT

public:
    explicit MainMenu(ScheduleService *svc, NLPService *nlp, QWidget *parent = nullptr);

public slots:
    void refreshSchedules();

signals:
    void petSelected(const QString &petId);
    void petScaleChanged(int scale);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void     setupUi(ScheduleService *svc, NLPService *nlp);
    QPushButton *makeNavBtn(const QString &text);
    QPushButton *makeContextBtn(const QString &text);
    QWidget     *makePlaceholder(const QString &text);
    QWidget     *makePetContext();
    QWidget     *makeSettingsContext(class SettingsPanel *settingsPanel);
    void         switchPage(int id);
    void         completePageSwitch(int id);
    void         playContextExit(int id, const std::function<void()> &finished);
    void         playContextEnter(int id);
    void         animateContextWidth(int targetWidth);
    void         prepareRightSurfaceEnter(int id);
    void         playRightSurfaceEnter(int id);
    void         setRightSurfaceTopAligned(bool aligned);
    void         updatePetContext(const QString &petId);
    void         updateRightSurfaceStyle(int pageId);

    QStackedWidget *m_stack  = nullptr;
    QStackedWidget *m_contextStack = nullptr;
    QButtonGroup   *m_navGrp = nullptr;
    QWidget        *m_contextPanel = nullptr;
    QWidget        *m_rightSurface = nullptr;
    QWidget        *m_titleBar = nullptr;
    QWidget        *m_petReveal = nullptr;
    QWidget        *m_scheduleReveal = nullptr;
    CalendarPanel  *m_calendarPanel = nullptr;
    QVariantAnimation *m_contextWidthAnim = nullptr;
    QVariantAnimation *m_rightSurfaceHeightAnim = nullptr;
    int             m_currentPage = 0;
    bool            m_pageSwitching = false;
    bool            m_dragging = false;
    QPoint          m_dragStartGlobal;
    QPoint          m_windowStartPos;
};

#endif // MAINMENU_H
