//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/pet/PetWidget.h"

#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

namespace {
struct GridInfo { int rows; int cols; };
// 已知非「单行横排」布局的精灵图；查不到则走 Animator 自动推断
GridInfo gridFor(const QString &petId, const QString &state) {
    if (petId == "Muelsyse" && state == "idle") return {6, 10};
    return {1, 0};
}
}

PetWidget::PetWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(128, 128);

    const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    move(screen.right() - 160, screen.bottom() - 160);

    connect(&m_animator, &Animator::frameChanged, this, QOverload<>::of(&QWidget::update));
    {
        const auto g = gridFor(m_petId, "idle");
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
        const QStringList states{"idle", "walk", "interact", "sleep"};
        if (idx < states.size()) emit animationRequested(states[idx]);
    });
}

void PetWidget::loadPet(const QString &petId) {
    m_petId = petId;
    const auto g = gridFor(m_petId, "idle");
    m_animator.load(m_petId, "idle", g.rows, g.cols);
}

void PetWidget::onStateChanged(const QString &state) {
    const auto g = gridFor(m_petId, state);
    m_animator.load(m_petId, state, g.rows, g.cols);
}

void PetWidget::showMainMenu(QPoint /*globalPos*/) {
    const QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    m_lastRightClick = center;
    m_mainMenu.popup(center, {{"日程", "📅"}, {"动作", "🎭"}, {"面板", "🪟"}, {"退出", "✕"}});
}

void PetWidget::showActionMenu(QPoint /*globalPos*/) {
    const QPoint center = mapToGlobal(QPoint(width() / 2, height() / 2));
    m_actionMenu.popup(center, {{"闲置", "😴"}, {"行走", "🚶"}, {"互动", "✨"}, {"睡眠", "🌙"}});
}

void PetWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawPixmap(rect(), m_animator.currentFrame());
}

void PetWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStart = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging  = false;
    } else if (event->button() == Qt::RightButton) {
        showMainMenu(event->globalPosition().toPoint());
    }
}

void PetWidget::mouseMoveEvent(QMouseEvent *event) {
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
    if (event->button() != Qt::LeftButton) return;

    if (m_dragging) {
        m_dragging = false;
        emit dragEnded();
    } else {
        emit interacted();
    }
}
