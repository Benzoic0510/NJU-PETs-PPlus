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
    move(screen.right() - width() - 32, screen.bottom() - height() - 32);

    connect(&m_animator, &Animator::frameChanged, this, QOverload<>::of(&QWidget::update));
    connect(&m_animator, &Animator::segmentFinished, this, &PetWidget::onSegmentFinished);

    {
        const auto g = SkinManifest::instance().gridFor(m_petId, "idle");
        m_animator.load(m_petId, "idle", g.rows, g.cols);
    }

    connect(&m_menu, &RadialMenu::triggered, this, &PetWidget::onMenuTriggered);
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
    const int side = 144 * m_petScale / 100;
    setFixedSize(side, side);
    update();
}

void PetWidget::setInteractionDisabled(bool disabled) {
    m_interactionDisabled = disabled;
    if (disabled) {
        m_dragging = false;
        m_menu.hide();
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
    m_menuPage = MenuPage::Main;
    m_menu.popup(center, {{"日程", "▤"},
                          {"动作", "✦"},
                          {"退出", "×"},
                          {"面板", "▦"}});
}

void PetWidget::showActionMenu(QPoint /*globalPos*/) {
    const QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    m_menuPage = MenuPage::Action;
    m_menu.popup(center, {{"待机", "◌"},
                          {"互动", "✧"},
                          {"返回", "↶"},
                          {"睡觉", "☽"}});
}

void PetWidget::showScheduleMenu(QPoint /*globalPos*/) {
    const QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    m_menuPage = MenuPage::Schedule;
    m_menu.popup(center, {{"添加", "＋"},
                          {"日历", "⌕"},
                          {"返回", "↶"},
                          {"近程", "◷"}});
}

void PetWidget::showQuitMenu(QPoint /*globalPos*/) {
    const QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    m_menuPage = MenuPage::Quit;
    m_menu.popup(center, {{"", "", true, false},
                          {"确认", "✓"},
                          {"取消", "↶"},
                          {"", "", true, false}});
}

void PetWidget::onMenuTriggered(int idx) {
    if (m_menuPage == MenuPage::Main) {
        if      (idx == 0) showScheduleMenu(m_lastRightClick);
        else if (idx == 1) showActionMenu(m_lastRightClick);
        else if (idx == 2) showQuitMenu(m_lastRightClick);
        else if (idx == 3) {
            m_menu.hide();
            emit showMainMenuRequested();
        }
        return;
    }

    if (m_menuPage == MenuPage::Schedule) {
        if (idx == 2) {
            showMainMenu(m_lastRightClick);
            return;
        }

        m_menu.hide();
        if      (idx == 0) emit nlpRequested();
        else if (idx == 1) emit showSchedulePanelRequested();
        else if (idx == 3) emit upcomingScheduleRequested();
        return;
    }

    if (m_menuPage == MenuPage::Action) {
        if (idx == 2) {
            showMainMenu(m_lastRightClick);
            return;
        }

        const QStringList states{"idle", "greet", "sleep"};
        m_menu.hide();
        if      (idx == 0) emit animationRequested(states[0]);
        else if (idx == 1) emit animationRequested(states[1]);
        else if (idx == 3) emit animationRequested(states[2]);
        return;
    }

    if (m_menuPage == MenuPage::Quit) {
        if (idx == 1) {
            m_menu.hide();
            emit quitRequested();
        } else if (idx == 2) {
            m_menu.hide();
        }
    }
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
