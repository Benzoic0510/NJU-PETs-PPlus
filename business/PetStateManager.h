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

public slots:
    void onDragStarted();
    void onDragEnded();
    void onInteract();
    void onRemind();
    void onSleep();
    void onWake();
    void onManualState(const QString &state);

signals:
    void stateChanged(const QString &state);

private slots:
    void onIdleTick();

private:
    void setState(const QString &state);

    QString m_state = "idle";
    QTimer  m_returnTimer;
    QTimer  m_idleTimer;
};

#endif //PETSTATEMANAGER_H
