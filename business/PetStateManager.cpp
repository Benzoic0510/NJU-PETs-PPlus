//
// Created by BenzoicAcid on 2026/5/12.
//

#include "business/PetStateManager.h"
#include "data/AppConfig.h"

#include <QRandomGenerator>

PetStateManager::PetStateManager(QObject *parent)
    : QObject(parent)
{
    m_returnTimer.setSingleShot(true);
    connect(&m_returnTimer, &QTimer::timeout, this, [this] { setState("idle"); });

    m_idleTimer.setInterval(40000);
    connect(&m_idleTimer, &QTimer::timeout, this, &PetStateManager::onIdleTick);

    m_sleepTimer.setSingleShot(true);
    m_sleepTimer.setInterval(AppConfig::instance().petSleepThresholdMins() * 60 * 1000);
    connect(&m_sleepTimer, &QTimer::timeout, this, &PetStateManager::onSleepTick);

    m_cooldownTimer.setSingleShot(true);
    connect(&m_cooldownTimer, &QTimer::timeout, this, [this]() { m_interactLocked = false; });

    setState("idle");
}

QString PetStateManager::currentState() const {
    return m_state;
}

void PetStateManager::onDragStarted() {
    m_returnTimer.stop();
    resetSleepTimer();
    setState("idle");
}

void PetStateManager::onDragEnded() {
    resetSleepTimer();
    setState("idle");
}

void PetStateManager::onInteract() {
    if (m_interactLocked) return;
    if (m_state == "sleep") return;
    m_interactLocked = true;
    m_cooldownTimer.stop();
    m_returnTimer.stop();
    resetSleepTimer();
    setState("greet");
    m_returnTimer.start(8000);
}

void PetStateManager::onRemind() {
    m_returnTimer.stop();
    if (m_state == "sleep") return;
    resetSleepTimer();
    setState("greet");
    m_returnTimer.start(5000);
}

void PetStateManager::onSleep() {
    m_returnTimer.stop();
    setState("sleep");
}

void PetStateManager::onWake() {
    setState("idle");
}

void PetStateManager::onManualState(const QString &state) {
    m_returnTimer.stop();
    setState(state);
    if (state != "sleep") m_returnTimer.start(8000);
}

void PetStateManager::onSleepWokeUp() {
    onWake();
}

void PetStateManager::setSleepThresholdMins(int mins) {
    m_sleepTimer.setInterval(qBound(1, mins, 60) * 60 * 1000);
    if (m_state != "sleep")
        resetSleepTimer();
}

void PetStateManager::onIdleTick() {
    // 30% greet 8s, 70% stay idle
    const int r = QRandomGenerator::global()->bounded(20);
    if (r < 6) {
        m_returnTimer.stop();
        setState("greet");
        m_returnTimer.start(8000);
    }
}

void PetStateManager::onSleepTick() {
    onSleep();
}

void PetStateManager::setState(const QString &state) {
    if (m_state == state) return;
    m_state = state;
    if (state == "sleep") {
        m_idleTimer.stop();
        m_sleepTimer.stop();
    } else {
        m_idleTimer.start();
        m_sleepTimer.start();
    }
    if (state == "idle")
        m_cooldownTimer.start(1000);
    emit stateChanged(m_state);
}

void PetStateManager::resetSleepTimer() {
    m_sleepTimer.start();
}
