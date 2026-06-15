//
// Created by LRYLH on 2026/6/8.
//

#ifndef SOUNDEFFECTSERVICE_H
#define SOUNDEFFECTSERVICE_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMap>
#include <QTimer>

class SoundEffectService : public QObject {
    Q_OBJECT
public:
    explicit SoundEffectService(QObject *parent = nullptr);
    void setMapping(const QMap<QString, QString> &mapping);

public slots:
    void play(const QString &eventKey);

private:
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audio = nullptr;
    QTimer        m_clearTimer;
    QMap<QString, QString> m_mapping;
    QString m_playingKey;
};

#endif // SOUNDEFFECTSERVICE_H