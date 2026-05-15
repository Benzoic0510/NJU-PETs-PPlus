//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/pet/BubbleWidget.h"
#include "presentation/common/Theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

BubbleWidget::BubbleWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(BubbleW);

    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText("说说你的日程…");
    m_edit->setStyleSheet(
        QString("QLineEdit {"
                "border: 1.5px solid %1;"
                "border-radius: 8px;"
                "padding: 4px 8px;"
                "font-size: 13px;"
                "background: white;"
                "}")
            .arg(Theme::Primary));
    connect(m_edit, &QLineEdit::returnPressed, this, &BubbleWidget::submit);
    m_edit->hide();

    m_loading = new QLabel("解析中…", this);
    m_loading->setStyleSheet(
        QString("QLabel { color: %1; font-size: 12px; background: transparent; }")
            .arg(Theme::TextTertiary));
    m_loading->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_loading->hide();

    m_output = new QLabel(this);
    m_output->setWordWrap(true);
    m_output->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_output->setStyleSheet(
        QString("QLabel { color: %1; font-size: 13px; background: transparent; }")
            .arg(Theme::TextPrimary));
    m_output->hide();

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &BubbleWidget::dismiss);

    hide();
}

void BubbleWidget::attachTo(QWidget *pet) {
    m_pet = pet;
}

// ── 状态切换 ──────────────────────────────────────────────────────────────────

void BubbleWidget::showInput() {
    m_inChat = true;
    m_timer.stop();
    m_loading->hide();
    m_edit->clear();
    m_edit->setEnabled(true);
    m_edit->show();
    // m_output 保持原状（有上一轮内容则继续显示在上方）
    updateLayout();
    show();
    m_edit->setFocus();
}

void BubbleWidget::showLoading() {
    m_edit->setEnabled(false);  // 保持可见，禁用输入等待回复
    m_loading->show();
    updateLayout();
}

void BubbleWidget::showResponse(const Schedule &s) {
    if (!m_inChat) return;
    m_loading->hide();
    m_edit->setEnabled(true);
    m_edit->clear();
    m_edit->hide();  // 日程已添加，关闭输入框

    m_output->setText(
        QString("已添加：%1\n%2 – %3")
            .arg(s.title)
            .arg(s.startTime.toString("MM月dd日 HH:mm"))
            .arg(s.endTime.toString("HH:mm")));
    m_output->show();
    m_hasOutput = true;
    updateLayout();
}

void BubbleWidget::showError(const QString &msg) {
    if (!m_inChat) return;
    // 非日程输入：显示提示，保留输入框（含上次文字）供重试
    m_loading->hide();
    m_output->setText(msg);
    m_output->show();
    m_hasOutput = true;
    m_edit->setEnabled(true);
    m_edit->clear();
    m_edit->show();
    updateLayout();
}

void BubbleWidget::showClarification(const QString &msg) {
    if (!m_inChat) return;
    // AI 追问：显示问题，清空输入框供用户补充信息
    m_loading->hide();
    m_output->setText(msg);
    m_output->show();
    m_hasOutput = true;
    m_edit->setEnabled(true);
    m_edit->clear();
    m_edit->show();
    updateLayout();
}

void BubbleWidget::showReminder(const Schedule &s) {
    if (m_inChat) return;  // 不打断正在进行的对话
    m_hasOutput = false;
    m_edit->hide();
    m_loading->hide();
    m_output->setText(QString("提醒：%1 即将开始").arg(s.title));
    m_output->show();
    updateLayout();
    show();
    m_timer.start(5000);
}

void BubbleWidget::dismiss() {
    m_hasOutput = false;
    m_inChat = false;
    m_edit->hide();
    m_loading->hide();
    m_output->hide();
    m_timer.stop();
    hide();
}

// ── 内部 ──────────────────────────────────────────────────────────────────────

void BubbleWidget::submit() {
    const QString text = m_edit->text().trimmed();
    if (text.isEmpty()) return;
    showLoading();
    emit submitted(text);
}

void BubbleWidget::updateLayout() {
    const int contentW = BubbleW - 2 * Pad;
    int y = Pad;
    m_dividerY = -1;

    // 输出在上
    if (!m_output->isHidden()) {
        m_output->setFixedWidth(contentW);
        int labelH = m_output->heightForWidth(contentW);
        if (labelH <= 0) labelH = m_output->sizeHint().height();
        labelH = qMax(labelH, 16);
        m_output->setGeometry(Pad, y, contentW, labelH);
        y += labelH + Pad;
    }

    // 分隔线：输出与后续内容之间
    const bool hasBelow = !m_loading->isHidden() || !m_edit->isHidden();
    if (!m_output->isHidden() && hasBelow) {
        m_dividerY = y - Pad / 2;
    }

    // 解析中提示
    if (!m_loading->isHidden()) {
        m_loading->setGeometry(Pad, y, contentW, 20);
        y += 20 + Pad;
    }

    // 输入在下
    if (!m_edit->isHidden()) {
        m_edit->setGeometry(Pad, y, contentW, 32);
        y += 32 + Pad;
    }

    setFixedSize(BubbleW, y + TailH);
    reposition();
    update();
}

void BubbleWidget::reposition() {
    if (!m_pet) return;
    const QPoint petGlobal = m_pet->mapToGlobal(QPoint(0, 0));
    const int x = petGlobal.x() + m_pet->width() / 2 - BubbleW / 2;
    const int y = petGlobal.y() - height() - 6;
    move(x, y);
}

void BubbleWidget::mousePressEvent(QMouseEvent *event) {
    // 成功添加后 input 被隐藏，点击气泡可以继续输入新日程
    if (m_inChat && m_hasOutput && m_edit->isHidden()) {
        showInput();
    }
    QWidget::mousePressEvent(event);
}

void BubbleWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int bubbleH = height() - TailH;

    // 气泡体 + 尾巴（WindingFill 取并集）
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(0, 0, BubbleW, bubbleH), CornerR, CornerR);

    const qreal cx = BubbleW / 2.0;
    QPolygonF tail;
    tail << QPointF(cx - 8, bubbleH - 2)
         << QPointF(cx + 8, bubbleH - 2)
         << QPointF(cx,     height());
    path.addPolygon(tail);
    path.closeSubpath();

    p.setPen(QPen(QColor(Theme::Border), 1.0));
    p.setBrush(QColor(255, 255, 255, 245));
    p.drawPath(path);

    // 输入框与输出之间的分隔线
    if (m_dividerY > 0) {
        p.setPen(QPen(QColor(Theme::Border), 1));
        p.drawLine(Pad, m_dividerY, BubbleW - Pad, m_dividerY);
    }
}
