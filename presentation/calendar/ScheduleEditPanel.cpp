//
// Created by BenzoicAcid on 2026/5/15.
//

#include "presentation/calendar/ScheduleEditPanel.h"
#include "presentation/common/Theme.h"

#include <QFrame>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>

ScheduleEditPanel::ScheduleEditPanel(ScheduleService *svc, NLPService *nlp, QWidget *parent)
    : QWidget(parent), m_svc(svc), m_nlp(nlp)
{
    setFixedWidth(320);
    setupUi();

    connect(m_nlp, &NLPService::parsed,             this, &ScheduleEditPanel::onNlpParsed);
    connect(m_nlp, &NLPService::parseFailed,         this, &ScheduleEditPanel::onNlpFailed);
    connect(m_nlp, &NLPService::clarificationNeeded, this, &ScheduleEditPanel::onNlpClarification);
}

void ScheduleEditPanel::setupUi() {
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(14, 14, 14, 12);
    outer->setSpacing(10);

    // ── 自然语言输入行 ────────────────────────────────────────
    auto *nlpRow = new QHBoxLayout;
    nlpRow->setSpacing(6);

    m_nlpEdit = new QLineEdit;
    m_nlpEdit->setPlaceholderText("用自然语言描述日程…");
    m_nlpEdit->setFixedHeight(30);
    m_nlpEdit->setStyleSheet(
        "QLineEdit { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 6px; padding: 0 8px; font-size: 12px; background: white; }"
        "QLineEdit:focus { border-color: " + Theme::Primary + "; }");

    m_parseBtn = new QPushButton("解析");
    m_parseBtn->setFixedSize(52, 30);
    m_parseBtn->setCursor(Qt::PointingHandCursor);
    m_parseBtn->setStyleSheet(
        "QPushButton { background: " + QString(Theme::Primary) + "; color: white;"
        "  border-radius: 6px; font-size: 12px; border: none; }"
        "QPushButton:hover { background: " + Theme::PrimaryDark + "; }"
        "QPushButton:disabled { opacity: 0.6; }");
    connect(m_parseBtn, &QPushButton::clicked,   this, &ScheduleEditPanel::onParse);
    connect(m_nlpEdit,  &QLineEdit::returnPressed, this, &ScheduleEditPanel::onParse);

    nlpRow->addWidget(m_nlpEdit, 1);
    nlpRow->addWidget(m_parseBtn);
    outer->addLayout(nlpRow);

    // 状态提示（错误/进度/成功）
    m_status = new QLabel;
    m_status->setWordWrap(true);
    m_status->setStyleSheet("font-size: 11px; color: " + QString(Theme::TextTertiary) + ";");
    m_status->hide();
    outer->addWidget(m_status);

    // ── 分隔线 ─────────────────────────────────────────────────
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background: " + QString(Theme::Border) + "; border: none;");
    outer->addWidget(sep);

    // ── 表单 ──────────────────────────────────────────────────
    const QString editSS =
        "QLineEdit { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 5px; padding: 2px 7px; font-size: 12px; }"
        "QLineEdit:focus { border-color: " + Theme::Primary + "; }";
    const QString dtSS =
        "QDateTimeEdit { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 5px; padding: 2px 6px; font-size: 12px; }"
        "QDateTimeEdit:focus { border-color: " + Theme::Primary + "; }";
    const QString labelSS =
        "font-size: 12px; color: " + QString(Theme::TextSecondary) + ";";

    auto *form = new QFormLayout;
    form->setSpacing(8);
    form->setContentsMargins(0, 0, 0, 0);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto makeLabel = [&](const QString &text) {
        auto *l = new QLabel(text);
        l->setStyleSheet(labelSS);
        return l;
    };

    m_titleEdit = new QLineEdit;
    m_titleEdit->setPlaceholderText("必填");
    m_titleEdit->setStyleSheet(editSS);
    form->addRow(makeLabel("标题"), m_titleEdit);

    m_startEdit = new QDateTimeEdit;
    m_startEdit->setDisplayFormat("yyyy-MM-dd  HH:mm");
    m_startEdit->setCalendarPopup(true);
    m_startEdit->setStyleSheet(dtSS);
    form->addRow(makeLabel("开始"), m_startEdit);

    m_endEdit = new QDateTimeEdit;
    m_endEdit->setDisplayFormat("yyyy-MM-dd  HH:mm");
    m_endEdit->setCalendarPopup(true);
    m_endEdit->setStyleSheet(dtSS);
    form->addRow(makeLabel("结束"), m_endEdit);

    m_locEdit = new QLineEdit;
    m_locEdit->setPlaceholderText("可选");
    m_locEdit->setStyleSheet(editSS);
    form->addRow(makeLabel("地点"), m_locEdit);

    m_remindBox = new QComboBox;
    m_remindBox->addItem("不提醒",      0);
    m_remindBox->addItem("提前 15 分钟", 15);
    m_remindBox->addItem("提前 30 分钟", 30);
    m_remindBox->addItem("提前 1 小时",  60);
    m_remindBox->addItem("提前 1 天",   1440);
    m_remindBox->setStyleSheet(
        "QComboBox { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 5px; padding: 2px 6px; font-size: 12px; }");
    form->addRow(makeLabel("提醒"), m_remindBox);

    outer->addLayout(form);

    // ── 操作按钮 ────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);

    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setFixedSize(64, 28);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 6px; font-size: 12px; background: none;"
        "  color: " + Theme::TextSecondary + "; }"
        "QPushButton:hover { background: " + Theme::BgSecondary + "; }");
    connect(cancelBtn, &QPushButton::clicked, this, [this]() {
        hide();
        emit dismissed();
    });

    auto *confirmBtn = new QPushButton("确认添加");
    confirmBtn->setFixedHeight(28);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(
        "QPushButton { background: " + QString(Theme::Primary) + "; color: white;"
        "  border-radius: 6px; font-size: 12px; border: none; padding: 0 12px; }"
        "QPushButton:hover { background: " + Theme::PrimaryDark + "; }");
    connect(confirmBtn, &QPushButton::clicked, this, &ScheduleEditPanel::onConfirm);

    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);
    outer->addLayout(btnRow);

    adjustSize();
}

void ScheduleEditPanel::showForNew(const QDate &date) {
    m_nlpEdit->clear();
    m_nlpEdit->setEnabled(true);
    m_nlpEdit->setPlaceholderText("用自然语言描述日程…");
    m_parseBtn->setEnabled(true);
    m_status->hide();
    m_titleEdit->clear();
    m_locEdit->clear();

    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime start(date, QTime(now.time().hour() + 1, 0));
    m_startEdit->setDateTime(start);
    m_endEdit->setDateTime(start.addSecs(3600));
    m_remindBox->setCurrentIndex(1);  // 15 min

    adjustSize();
    show();
    raise();
    m_nlpEdit->setFocus();
}

void ScheduleEditPanel::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 背景白底 + 边框
    p.setPen(QPen(QColor(Theme::Border), 1));
    p.setBrush(QColor(255, 255, 255, 250));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
}

void ScheduleEditPanel::fillForm(const Schedule &s) {
    if (!s.title.isEmpty())    m_titleEdit->setText(s.title);
    if (s.startTime.isValid()) m_startEdit->setDateTime(s.startTime);
    if (s.endTime.isValid())   m_endEdit->setDateTime(s.endTime);
    if (!s.location.isEmpty()) m_locEdit->setText(s.location);
    for (int i = 0; i < m_remindBox->count(); ++i) {
        if (m_remindBox->itemData(i).toInt() == s.remindMins) {
            m_remindBox->setCurrentIndex(i);
            break;
        }
    }
}

void ScheduleEditPanel::setStatus(const QString &msg, bool isError) {
    m_status->setText(msg);
    m_status->setStyleSheet(
        QString("font-size: 11px; color: %1;")
            .arg(isError ? "#D85A30" : Theme::TextTertiary));
    m_status->show();
    adjustSize();
    raise();
}

// ── NLP 槽 ─────────────────────────────────────────────────────────────────────

void ScheduleEditPanel::onParse() {
    const QString text = m_nlpEdit->text().trimmed();
    if (text.isEmpty()) return;
    m_parseBtn->setEnabled(false);
    m_nlpEdit->setEnabled(false);
    setStatus("解析中…");
    m_nlp->parse(text);
}

void ScheduleEditPanel::onNlpParsed(const Schedule &s) {
    if (!isVisible()) return;
    m_parseBtn->setEnabled(true);
    m_nlpEdit->setEnabled(true);
    setStatus("已填入表单，请确认或修改后点击「确认添加」。");
    fillForm(s);
}

void ScheduleEditPanel::onNlpFailed(const QString &reason) {
    if (!isVisible()) return;
    m_parseBtn->setEnabled(true);
    m_nlpEdit->setEnabled(true);
    setStatus(reason, true);
}

void ScheduleEditPanel::onNlpClarification(const QString &question) {
    if (!isVisible()) return;
    m_parseBtn->setEnabled(true);
    m_nlpEdit->setEnabled(true);
    m_nlpEdit->clear();
    m_nlpEdit->setPlaceholderText("请补充信息…");
    m_nlpEdit->setFocus();
    setStatus(question);
}

// ── 确认提交 ─────────────────────────────────────────────────────────────────────

void ScheduleEditPanel::onConfirm() {
    const QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        setStatus("标题不能为空", true);
        return;
    }
    if (m_endEdit->dateTime() <= m_startEdit->dateTime()) {
        setStatus("结束时间必须晚于开始时间", true);
        return;
    }

    Schedule s;
    s.title      = title;
    s.startTime  = m_startEdit->dateTime();
    s.endTime    = m_endEdit->dateTime();
    s.location   = m_locEdit->text().trimmed();
    s.remindMins = m_remindBox->currentData().toInt();

    if (m_svc->addSchedule(s) < 0) {
        setStatus("该时间段与已有日程冲突，请调整时间。", true);
    } else {
        hide();
        emit dismissed();
    }
}
