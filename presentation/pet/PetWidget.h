//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef PETWIDGET_H
#define PETWIDGET_H

#include "presentation/pet/Animator.h"
#include "presentation/pet/RadialMenu.h"

#include <QWidget>

class PetWidget : public QWidget {
    Q_OBJECT

public:
    explicit PetWidget(QWidget *parent = nullptr);

    void loadPet(const QString &petId);

signals:
    void dragStarted();
    void dragEnded();
    void interacted();
    void nlpRequested();
    void animationRequested(const QString &state);
    void showMainMenuRequested();
    void quitRequested();

public slots:
    void onStateChanged(const QString &state);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void showMainMenu(QPoint globalPos);
    void showActionMenu(QPoint globalPos);

    Animator   m_animator;
    RadialMenu m_mainMenu;
    RadialMenu m_actionMenu;
    QString    m_petId    = "Muelsyse";
    QPoint     m_dragStart;
    bool       m_dragging = false;
    QPoint     m_lastRightClick;

    static constexpr int DragThreshold = 4;
};

#endif // PETWIDGET_H
