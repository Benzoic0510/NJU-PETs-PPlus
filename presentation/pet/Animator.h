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

    // rows/cols 描述 spritesheet 的网格布局；cols<=0 时按单行横排自动推断（width/height）
    void    load(const QString &petId, const QString &state, int rows = 1, int cols = 0);
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
    int     m_rows       = 1;
    int     m_cols       = 1;
    int     m_frameCount = 1;
    int     m_frameIndex = 0;
    double  m_frameW     = 128.0;
    double  m_frameH     = 128.0;
    int     m_frameSize  = 128;   // 仅占位符使用
    bool    m_hasSheet   = false;
    QTimer  m_timer;
};

#endif // ANIMATOR_H
