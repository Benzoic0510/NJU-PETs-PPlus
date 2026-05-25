//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef PETWIDGET_H
#define PETWIDGET_H

#include "presentation/pet/Animator.h"
#include "presentation/pet/RadialMenu.h"

#include <QWidget>

class PetWidget : public QWidget {
    Q_OBJECT

public:
    explicit PetWidget(QWidget *parent = nullptr);

    void loadPet(const QString &petId);

signals:
    void dragStarted();
    void dragEnded();
    void interacted();
    void nlpRequested();
    void animationRequested(const QString &state);
    void showMainMenuRequested();
    void quitRequested();
    void sleepWokeUp();

public slots:
    void onStateChanged(const QString &state);
    void setPetScale(int scale);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void showMainMenu(QPoint globalPos);
    void showActionMenu(QPoint globalPos);
    void startSleepSequence();
    void onSegmentFinished();

    enum SleepPhase { SleepNone, SleepFalling, SleepLooping, SleepWaking };

    Animator   m_animator;
    RadialMenu m_mainMenu;
    RadialMenu m_actionMenu;
    QString    m_petId       = "Muelsyse";
    int        m_petScale    = 100;
    QPoint     m_dragStart;
    bool       m_dragging    = false;
    QPoint     m_lastRightClick;
    SleepPhase m_sleepPhase  = SleepNone;
    bool       m_wakePending = false;

    static constexpr int DragThreshold = 4;
    static constexpr int kFallingLast  = 14;   // frames 0-14: 入睡
    static constexpr int kLoopStart    = 15;   // frames 15-44: 睡眠循环
    static constexpr int kLoopLast     = 44;
    static constexpr int kWakeStart    = 45;   // frames 45-59: 醒来
    static constexpr int kWakeLast     = 59;
};

#endif // PETWIDGET_H
