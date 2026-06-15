//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef PETSTATEMANAGER_H
#define PETSTATEMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>

class PetStateManager : public QObject {
    Q_OBJECT

public:
    explicit PetStateManager(QObject *parent = nullptr);

    QString currentState() const;

    bool isInteractLocked() const { return m_interactLocked; }

public slots:
    void onDragStarted();
    void onDragEnded();
    void onInteract();
    void onRemind();
    void onSleep();
    void onWake();
    void onManualState(const QString &state);
    void onSleepWokeUp();
    void setSleepThresholdMins(int mins);

signals:
    void stateChanged(const QString &state);

private slots:
    void onIdleTick();
    void onSleepTick();

private:
    void setState(const QString &state);
    void resetSleepTimer();

    QString m_state = "idle";
    QTimer  m_returnTimer;
    QTimer  m_idleTimer;
    QTimer  m_sleepTimer;
    QTimer  m_cooldownTimer;
    bool    m_interactLocked = false;
};

#endif //PETSTATEMANAGER_H
