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

    m_closeBtn = new QPushButton("×", this);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setStyleSheet(
        "QPushButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: " + QString(Theme::TextTertiary) + ";"
        "  font-size: 16px;"
        "  font-weight: 400;"
        "  padding: 0;"
        "}"
        "QPushButton:hover {"
        "  color: " + QString(Theme::TextSecondary) + ";"
        "}");
    connect(m_closeBtn, &QPushButton::clicked, this, &BubbleWidget::dismiss);

    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText("说说你的日程…");
    m_edit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 10px;"
        "  padding: 6px 10px;"
        "  font-size: 13px;"
        "  color: " + Theme::TextPrimary + ";"
        "  background: " + Theme::BgSecondary + ";"
        "  selection-background-color: " + Theme::PrimaryMid + ";"
        "}"
        "QLineEdit:focus {"
        "  border-color: " + Theme::PrimaryMid + ";"
        "  background: " + Theme::BgPrimary + ";"
        "}");
    connect(m_edit, &QLineEdit::returnPressed, this, &BubbleWidget::submit);
    m_edit->hide();

    m_loading = new QLabel("解析中…", this);
    m_loading->setStyleSheet(
        QString("QLabel { color: %1; font-size: 12px; background: transparent; }")
            .arg(Theme::TextSecondary));
    m_loading->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_loading->hide();

    m_output = new QLabel(this);
    m_output->setWordWrap(true);
    m_output->setTextFormat(Qt::PlainText);
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

    m_output->setTextFormat(Qt::PlainText);
    m_output->setText(
        s.isDDL
            ? QString("已添加 DDL：%1\n截止 %2")
                  .arg(s.title)
                  .arg(s.startTime.toString("MM月dd日 HH:mm"))
            : QString("已添加：%1\n%2 – %3")
                  .arg(s.title)
                  .arg(s.startTime.toString("MM月dd日 HH:mm"))
                  .arg(s.endTime.toString("HH:mm")));
    m_output->show();
    m_hasOutput = true;
    updateLayout();
}

void BubbleWidget::showReminder(const Schedule &s) {
    m_timer.stop();
    m_inChat = false;
    m_loading->hide();
    m_edit->hide();
    m_output->setTextFormat(Qt::PlainText);
    m_output->setText(
        s.isDDL
            ? QString("DDL 提醒\n%1\n截止 %2")
                  .arg(s.title)
                  .arg(s.startTime.toString("MM月dd日 HH:mm"))
            : QString("日程提醒\n%1\n%2 – %3")
                  .arg(s.title)
                  .arg(s.startTime.toString("MM月dd日 HH:mm"))
                  .arg(s.endTime.toString("HH:mm")));
    m_output->show();
    m_hasOutput = true;
    updateLayout();
    show();
    m_timer.start(8000);
}

void BubbleWidget::showUpcoming(const QVector<Schedule> &schedules) {
    m_inChat = false;
    m_hasOutput = true;
    m_timer.stop();
    m_loading->hide();
    m_edit->hide();
    m_output->setTextFormat(Qt::RichText);

    if (schedules.isEmpty()) {
        m_output->setText(
            "<div style='font-size:13px; color:#1A1A2E;'>"
            "<div style='font-weight:700; margin-bottom:6px;'>即将到来</div>"
            "<div style='color:#8888AA; padding-top:4px;'>接下来暂时没有日程。</div>"
            "</div>");
    } else {
        QString html =
            "<div style='font-size:13px; color:#1A1A2E;'>"
            "<div style='font-weight:700; margin-bottom:8px;'>即将到来</div>"
            "<table cellspacing='0' cellpadding='0' width='100%'>";

        for (const Schedule &s : schedules) {
            const QString time = s.isDDL
                ? QString("DDL %1").arg(s.startTime.toString("MM/dd HH:mm"))
                : s.startTime.toString("MM/dd HH:mm");
            QString meta = s.location.trimmed();
            if (!s.isDDL && s.endTime.isValid())
                meta = meta.isEmpty()
                    ? QString("至 %1").arg(s.endTime.toString("HH:mm"))
                    : QString("%1 · 至 %2").arg(meta, s.endTime.toString("HH:mm"));

            html +=
                "<tr>"
                "<td style='width:74px; padding:7px 8px 7px 0; color:#534AB7; font-size:12px; white-space:nowrap; vertical-align:top;'>"
                + time.toHtmlEscaped() +
                "</td>"
                "<td style='padding:6px 0 7px 0; vertical-align:top;'>"
                "<div style='font-weight:600; color:#1A1A2E;'>"
                + s.title.toHtmlEscaped() +
                "</div>";
            if (!meta.isEmpty()) {
                html +=
                    "<div style='font-size:12px; color:#8888AA; margin-top:2px;'>"
                    + meta.toHtmlEscaped() +
                    "</div>";
            }
            html += "</td></tr>";
        }
        html += "</table></div>";
        m_output->setText(html);
    }

    m_output->show();
    updateLayout();
    show();
}

void BubbleWidget::showError(const QString &msg) {
    if (!m_inChat) return;
    // 非日程输入：显示提示，保留输入框（含上次文字）供重试
    m_loading->hide();
    m_output->setTextFormat(Qt::PlainText);
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
    m_output->setTextFormat(Qt::PlainText);
    m_output->setText(msg);
    m_output->show();
    m_hasOutput = true;
    m_edit->setEnabled(true);
    m_edit->clear();
    m_edit->show();
    updateLayout();
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
    const int contentW = BubbleW - 2 * Pad - CloseBtnSize - 4;
    int y = Pad;
    m_dividerY = -1;

    m_closeBtn->setGeometry(BubbleW - Pad - CloseBtnSize, Pad - 2, CloseBtnSize, CloseBtnSize);
    m_closeBtn->raise();

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
        m_edit->setGeometry(Pad, y, contentW, 36);
        y += 36 + Pad;
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

    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, BubbleW, bubbleH), CornerR, CornerR);

    QColor bubbleBg(Theme::BgPrimary);
    bubbleBg.setAlpha(238);
    QColor border(Theme::Border);
    border.setAlpha(180);
    p.setPen(QPen(border, 1.0));
    p.setBrush(bubbleBg);
    p.drawPath(path);

    // 输入框与输出之间的分隔线
    if (m_dividerY > 0) {
        QColor divider(Theme::Border);
        divider.setAlpha(150);
        p.setPen(QPen(divider, 1));
        p.drawLine(Pad, m_dividerY, BubbleW - Pad, m_dividerY);
    }
}
