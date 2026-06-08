//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/pet/PetWidget.h"
#include "presentation/pet/SkinManifest.h"
#include "data/AppConfig.h"

#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

PetWidget::PetWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setPetScale(AppConfig::instance().petScale());
    setInteractionDisabled(AppConfig::instance().petInteractionDisabled());

    const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    move(screen.right() - 160, screen.bottom() - 160);

    connect(&m_animator, &Animator::frameChanged, this, QOverload<>::of(&QWidget::update));
    connect(&m_animator, &Animator::segmentFinished, this, &PetWidget::onSegmentFinished);

    {
        const auto g = SkinManifest::instance().gridFor(m_petId, "idle");
        m_animator.load(m_petId, "idle", g.rows, g.cols);
    }

    // 主菜单：日程 / 动作 / 面板 / 退出
    connect(&m_mainMenu, &RadialMenu::triggered, this, [this](int idx) {
        if      (idx == 0) emit nlpRequested();
        else if (idx == 1) showActionMenu(m_lastRightClick);
        else if (idx == 2) emit showMainMenuRequested();
        else if (idx == 3) emit quitRequested();
    });

    // 动作子菜单
    connect(&m_actionMenu, &RadialMenu::triggered, this, [this](int idx) {
        const QStringList states{"idle", "greet", "sleep"};
        if (idx < states.size()) emit animationRequested(states[idx]);
    });
}

void PetWidget::loadPet(const QString &petId) {
    m_petId = petId;
    m_sleepPhase  = SleepNone;
    m_wakePending = false;
    const auto g = SkinManifest::instance().gridFor(m_petId, "idle");
    m_animator.load(m_petId, "idle", g.rows, g.cols);
}

void PetWidget::setPetScale(int scale) {
    m_petScale = qBound(60, scale, 180);
    const int side = 128 * m_petScale / 100;
    setFixedSize(side, side);
    update();
}

void PetWidget::setInteractionDisabled(bool disabled) {
    m_interactionDisabled = disabled;
    if (disabled) {
        m_dragging = false;
        m_mainMenu.hide();
        m_actionMenu.hide();
    }
}

void PetWidget::onStateChanged(const QString &state) {
    if (state == "sleep") {
        startSleepSequence();
    } else {
        m_sleepPhase  = SleepNone;
        m_wakePending = false;
        m_animator.resetSegment();
        const auto g = SkinManifest::instance().gridFor(m_petId, state);
        m_animator.load(m_petId, state, g.rows, g.cols);
    }
}

void PetWidget::startSleepSequence() {
    m_sleepPhase  = SleepFalling;
    m_wakePending = false;
    const auto seg = SkinManifest::instance().sleepSegmentsFor(m_petId);
    const auto g   = SkinManifest::instance().gridFor(m_petId, "sleep");
    m_animator.load(m_petId, "sleep", g.rows, g.cols);
    m_animator.playOnce(seg.fallingStart, seg.fallingEnd + 1);
}

void PetWidget::onSegmentFinished() {
    const auto seg = SkinManifest::instance().sleepSegmentsFor(m_petId);

    switch (m_sleepPhase) {
    case SleepFalling:
        m_sleepPhase = SleepLooping;
        m_animator.setSegment(seg.loopStart, seg.loopEnd + 1);
        break;
    case SleepLooping:
        if (m_wakePending) {
            m_sleepPhase  = SleepWaking;
            m_wakePending = false;
            m_animator.playOnce(seg.wakeStart, seg.wakeEnd + 1);
        }
        break;
    case SleepWaking:
        m_sleepPhase = SleepNone;
        m_animator.resetSegment();
        emit sleepWokeUp();
        break;
    case SleepNone:
        break;
    }
}

void PetWidget::showMainMenu(QPoint /*globalPos*/) {
    const QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    m_lastRightClick = center;
    m_mainMenu.popup(center, {{"日程", ":/icons/schedule.svg"}, {"动作", ":/icons/action.svg"}, {"面板", ":/icons/panel.svg"}, {"退出", ":/icons/quit.svg"}});
}

void PetWidget::showActionMenu(QPoint /*globalPos*/) {
    const QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    m_actionMenu.popup(center, {{"待机", "😴"}, {"互动", "✨"}, {"睡觉", "🌙"}});
}

void PetWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawPixmap(rect(), m_animator.currentFrame());
}

void PetWidget::mousePressEvent(QMouseEvent *event) {
    if (m_interactionDisabled) {
        event->ignore();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_dragStart = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging  = false;
    } else if (event->button() == Qt::RightButton) {
        showMainMenu(event->globalPosition().toPoint());
    }
}

void PetWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_interactionDisabled) {
        event->ignore();
        return;
    }

    if (!(event->buttons() & Qt::LeftButton)) return;

    const QPoint delta = event->globalPosition().toPoint() - frameGeometry().topLeft() - m_dragStart;
    if (!m_dragging && delta.manhattanLength() >= DragThreshold) {
        m_dragging = true;
        emit dragStarted();
    }

    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragStart);
    }
}

void PetWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_interactionDisabled) {
        event->ignore();
        return;
    }

    if (event->button() != Qt::LeftButton) return;

    if (m_dragging) {
        m_dragging = false;
        emit dragEnded();
        return;
    }

    // 睡眠中点击：入睡/醒来阶段忽略，循环阶段请求唤醒
    if (m_sleepPhase == SleepFalling || m_sleepPhase == SleepWaking) {
        return;
    }
    if (m_sleepPhase == SleepLooping) {
        m_wakePending = true;
        return;
    }

    emit interacted();
}
