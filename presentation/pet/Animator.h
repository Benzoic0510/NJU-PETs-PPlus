//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QObject>
#include <QPixmap>
#include <QTimer>

class Animator : public QObject {
    Q_OBJECT

public:
    explicit Animator(QObject *parent = nullptr);

    void    load(const QString &petId, const QString &state);
    void    setFps(int fps);
    QPixmap currentFrame() const;

signals:
    void frameChanged();

private slots:
    void nextFrame();

private:
    QPixmap makePlaceholder() const;

    QString m_petId;
    QString m_state;
    QPixmap m_sheet;
    int     m_frameCount = 1;
    int     m_frameIndex = 0;
    int     m_frameSize  = 128;
    bool    m_hasSheet   = false;
    QTimer  m_timer;
};

#endif // ANIMATOR_H
