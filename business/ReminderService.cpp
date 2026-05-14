//
// Created by BenzoicAcid on 2026/5/12.
//

#include "business/ReminderService.h"
#include "data/AppConfig.h"

#include <QDateTime>

ReminderService::ReminderService(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &ReminderService::scan);
}

void ReminderService::start(int intervalMs) {
    m_timer.start(intervalMs);
}

void ReminderService::stop() {
    m_timer.stop();
}

void ReminderService::scan() {
    if (!AppConfig::instance().reminderEnabled()) return;
    const QDateTime now = QDateTime::currentDateTime();
    const auto schedules = m_repo.getAll();

    for (const auto &s : schedules) {
        if (s.remindMins <= 0)          continue;
        if (m_notified.contains(s.id))  continue;

        const qint64 secsToStart = now.secsTo(s.startTime);
        if (secsToStart >= 0 && secsToStart <= static_cast<qint64>(s.remindMins) * 60) {
            m_notified.insert(s.id);
            emit remind(s);
        }
    }
}
