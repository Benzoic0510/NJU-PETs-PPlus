//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef BUBBLEWIDGET_H
#define BUBBLEWIDGET_H

#include "data/models/Schedule.h"

#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QWidget>

class BubbleWidget : public QWidget {
    Q_OBJECT

public:
    explicit BubbleWidget(QWidget *parent = nullptr);

    void attachTo(QWidget *pet);
    void showInput();
    void showResponse(const Schedule &s);
    void showError(const QString &msg);
    void showClarification(const QString &msg);
    void showReminder(const Schedule &s);
    void dismiss();

signals:
    void submitted(const QString &text);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    void submit();
    void showLoading();
    void updateLayout();
    void reposition();

    QWidget   *m_pet     = nullptr;
    QLineEdit *m_edit    = nullptr;
    QLabel    *m_loading = nullptr;
    QLabel    *m_output  = nullptr;
    QTimer     m_timer;

    bool m_hasOutput = false;
    bool m_inChat    = false;
    int  m_dividerY  = -1;

    static constexpr int BubbleW = 260;
    static constexpr int TailH   = 12;
    static constexpr int Pad     = 12;
    static constexpr int CornerR = 12;
};

#endif // BUBBLEWIDGET_H
