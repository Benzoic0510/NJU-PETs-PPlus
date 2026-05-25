//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/pet/Animator.h"
#include "presentation/common/Theme.h"

#include <QPainter>
#include <QPainterPath>

Animator::Animator(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &Animator::nextFrame);
    setFps(8);
}

void Animator::load(const QString &petId, const QString &state, int rows, int cols) {
    m_petId = petId;
    m_state = state;
    m_frameIndex = 0;
    m_oneShot    = false;
    m_segStart   = 0;
    m_segEnd     = 0;

    const QString path = ":/sprites/" + petId + "/" + state + ".png";
    QPixmap sheet(path);

    if (!sheet.isNull()) {
        if (cols <= 0) {
            rows = 1;
            cols = sheet.width() / sheet.height();
            if (cols <= 0) cols = 1;
        }
        m_sheet      = sheet;
        m_rows       = rows;
        m_cols       = cols;
        m_frameCount = rows * cols;
        m_frameW     = double(sheet.width())  / cols;
        m_frameH     = double(sheet.height()) / rows;
        m_hasSheet   = true;
    } else {
        m_hasSheet   = false;
        m_rows       = 1;
        m_cols       = 1;
        m_frameCount = 1;
    }

    emit frameChanged();
}

void Animator::setFps(int fps) {
    m_timer.start(fps > 0 ? 1000 / fps : 1000);
}

QPixmap Animator::currentFrame() const {
    if (m_hasSheet) {
        const int col = m_frameIndex % m_cols;
        const int row = m_frameIndex / m_cols;
        const int x = qRound(col * m_frameW);
        const int y = qRound(row * m_frameH);
        const int w = qRound((col + 1) * m_frameW) - x;
        const int h = qRound((row + 1) * m_frameH) - y;
        return m_sheet.copy(x, y, w, h);
    }
    return makePlaceholder();
}

void Animator::setSegment(int start, int end) {
    m_segStart = start;
    m_segEnd   = end;
    m_oneShot  = false;
    if (m_frameIndex < start || m_frameIndex >= end)
        m_frameIndex = start;
    m_timer.start();
    emit frameChanged();
}

void Animator::playOnce(int start, int end) {
    m_segStart = start;
    m_segEnd   = end;
    m_oneShot  = true;
    m_frameIndex = start;
    m_timer.start();
    emit frameChanged();
}

void Animator::resetSegment() {
    m_segStart = 0;
    m_segEnd   = 0;
    m_oneShot  = false;
    m_timer.start();
    emit frameChanged();
}

void Animator::nextFrame() {
    const int start = (m_segEnd > 0) ? m_segStart : 0;
    const int end   = (m_segEnd > 0) ? m_segEnd   : m_frameCount;

    m_frameIndex++;
    if (m_frameIndex >= end) {
        if (m_oneShot) {
            m_frameIndex = end - 1;
            m_timer.stop();
            emit segmentFinished();
        } else {
            m_frameIndex = start;
            emit segmentFinished();
        }
    }
    emit frameChanged();
}

QPixmap Animator::makePlaceholder() const {
    QPixmap px(m_frameSize, m_frameSize);
    px.fill(Qt::transparent);

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(Theme::PrimaryBg));
    p.drawEllipse(px.rect().adjusted(4, 4, -4, -4));

    p.setPen(QColor(Theme::Primary));
    p.setFont(QFont("Segoe UI", 32));
    p.drawText(px.rect(), Qt::AlignCenter, "✦");

    return px;
}
