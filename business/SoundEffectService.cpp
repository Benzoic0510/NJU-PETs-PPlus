//
// Created by BenzoicAcid on 2026/6/8.
//

#include "business/SoundEffectService.h"
#include <QUrl>

SoundEffectService::SoundEffectService(QObject *parent) : QObject(parent) {
    m_player = new QMediaPlayer(this);
    m_audio = new QAudioOutput(this);
    m_player->setAudioOutput(m_audio);
    m_clearTimer.setSingleShot(true);
    connect(&m_clearTimer, &QTimer::timeout, this, [this]() { m_playingKey.clear(); });
}

void SoundEffectService::setMapping(const QMap<QString, QString> &mapping) {
    m_mapping = mapping;
    m_playingKey.clear();
}

void SoundEffectService::play(const QString &eventKey) {
    if (m_playingKey == eventKey)
        return;

    const QString path = m_mapping.value(eventKey);
    if (!path.isEmpty()) {
        m_playingKey = eventKey;
        m_clearTimer.start(3000);
        if (path.startsWith(":/")) {
            m_player->setSource(QUrl("qrc" + path));
        } else {
            m_player->setSource(QUrl::fromLocalFile(path));
        }
        m_player->play();
    }
}