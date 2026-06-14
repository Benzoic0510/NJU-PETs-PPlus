//
// Created by BenzoicAcid on 2026/6/8.
//

#include "business/SoundEffectService.h"
#include <QUrl>

SoundEffectService::SoundEffectService(QObject *parent) : QObject(parent) {
    m_player = new QMediaPlayer(this);
    m_audio = new QAudioOutput(this);
    m_player->setAudioOutput(m_audio);
}

void SoundEffectService::setMapping(const QMap<QString, QString> &mapping) {
    m_mapping = mapping;
}

void SoundEffectService::play(const QString &eventKey) {
    const QString path = m_mapping.value(eventKey);
    if (!path.isEmpty()) {
        m_player->stop();
        if (path.startsWith(":/")) {
            m_player->setSource(QUrl("qrc" + path));
        } else {
            m_player->setSource(QUrl::fromLocalFile(path));
        }
        m_player->play();
    }
}