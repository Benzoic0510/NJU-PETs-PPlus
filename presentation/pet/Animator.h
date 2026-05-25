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

    void    load(const QString &petId, const QString &state, int rows = 1, int cols = 0);
    void    setFps(int fps);
    QPixmap currentFrame() const;

    void    setSegment(int start, int end);  // loop within [start, end)
    void    playOnce(int start, int end);    // play [start, end) once, stop, emit segmentFinished
    void    resetSegment();                  // back to full range looping

signals:
    void frameChanged();
    void segmentFinished();

private slots:
    void nextFrame();

private:
    QPixmap makePlaceholder() const;

    QString m_petId;
    QString m_state;
    QPixmap m_sheet;
    int     m_rows       = 1;
    int     m_cols       = 1;
    int     m_frameCount = 1;
    int     m_frameIndex = 0;
    double  m_frameW     = 128.0;
    double  m_frameH     = 128.0;
    int     m_frameSize  = 128;
    bool    m_hasSheet   = false;
    bool    m_oneShot    = false;
    int     m_segStart   = 0;
    int     m_segEnd     = 0;
    QTimer  m_timer;
};

#endif // ANIMATOR_H
