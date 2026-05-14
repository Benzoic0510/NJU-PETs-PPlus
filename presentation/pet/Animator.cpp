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

void Animator::load(const QString &petId, const QString &state) {
    m_petId = petId;
    m_state = state;
    m_frameIndex = 0;

    const QString path = ":/sprites/" + petId + "_" + state + ".png";
    QPixmap sheet(path);

    if (!sheet.isNull()) {
        m_sheet      = sheet;
        m_frameSize  = sheet.height();
        m_frameCount = sheet.width() / m_frameSize;
        m_hasSheet   = true;
    } else {
        m_hasSheet   = false;
        m_frameCount = 1;
    }

    emit frameChanged();
}

void Animator::setFps(int fps) {
    m_timer.start(fps > 0 ? 1000 / fps : 1000);
}

QPixmap Animator::currentFrame() const {
    if (m_hasSheet) {
        return m_sheet.copy(m_frameIndex * m_frameSize, 0, m_frameSize, m_frameSize);
    }
    return makePlaceholder();
}

void Animator::nextFrame() {
    m_frameIndex = (m_frameIndex + 1) % m_frameCount;
    emit frameChanged();
}

QPixmap Animator::makePlaceholder() const {
    QPixmap px(m_frameSize, m_frameSize);
    px.fill(Qt::transparent);

    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    // 圆形背景
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(Theme::PrimaryBg));
    p.drawEllipse(px.rect().adjusted(4, 4, -4, -4));

    // 中心文字
    p.setPen(QColor(Theme::Primary));
    p.setFont(QFont("Segoe UI", 28));
    p.drawText(px.rect(), Qt::AlignCenter, "🐋");

    return px;
}
