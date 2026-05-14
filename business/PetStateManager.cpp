//
// Created by BenzoicAcid on 2026/5/12.
//

#include "business/PetStateManager.h"

PetStateManager::PetStateManager(QObject *parent)
    : QObject(parent)
{
    m_returnTimer.setSingleShot(true);
    connect(&m_returnTimer, &QTimer::timeout, this, [this] { setState("idle"); });
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

void PetStateManager::setState(const QString &state) {
    if (m_state == state) return;
    m_state = state;
    emit stateChanged(m_state);
}
