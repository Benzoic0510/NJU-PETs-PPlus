//
// Created by BenzoicAcid on 2026/5/12.
//

#include "business/PetStateManager.h"

#include <QRandomGenerator>

PetStateManager::PetStateManager(QObject *parent)
    : QObject(parent)
{
    m_returnTimer.setSingleShot(true);
    connect(&m_returnTimer, &QTimer::timeout, this, [this] { setState("idle"); });

    m_idleTimer.setInterval(40000);
    connect(&m_idleTimer, &QTimer::timeout, this, &PetStateManager::onIdleTick);
    m_idleTimer.start();
}

QString PetStateManager::currentState() const {
    return m_state;
}

void PetStateManager::onDragStarted() {
    m_returnTimer.stop();
    setState("drag");
}

void PetStateManager::onDragEnded() {
    setState("idle");
}

void PetStateManager::onInteract() {
    m_returnTimer.stop();
    setState("interact");
    m_returnTimer.start(3000);
}

void PetStateManager::onRemind() {
    m_returnTimer.stop();
    setState("interact");
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

void PetStateManager::onIdleTick() {
    // 30% walk 8s, 15% sleep, 55% stay idle
    const int r = QRandomGenerator::global()->bounded(20);
    if (r < 6) {
        m_returnTimer.stop();
        setState("walk");
        m_returnTimer.start(8000);
    } else if (r < 9) {
        onSleep();
    }
}

void PetStateManager::setState(const QString &state) {
    if (m_state == state) return;
    m_state = state;
    if (state == "idle")
        m_idleTimer.start();
    else
        m_idleTimer.stop();
    emit stateChanged(m_state);
}
