//
// Created by BenzoicAcid on 2026/5/15.
//

#include "presentation/calendar/ScheduleEditPanel.h"
#include "presentation/common/Theme.h"

#include <QAbstractItemView>
#include <QCalendarWidget>
#include <QEvent>
#include <QFrame>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QPainter>
#include <QTextCharFormat>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

class ThemedCalendarWidget : public QCalendarWidget {
public:
    explicit ThemedCalendarWidget(QWidget *parent = nullptr)
        : QCalendarWidget(parent)
    {
        setMouseTracking(true);
    }

    void enableHoverTracking() {
        if (m_calendarView) return;
        m_calendarView = findChild<QAbstractItemView *>("qt_calendar_calendarview");
        if (!m_calendarView) return;
        m_calendarView->setFocusPolicy(Qt::NoFocus);
        m_calendarView->setSelectionMode(QAbstractItemView::NoSelection);
        m_calendarView->setCurrentIndex(QModelIndex());
        if (m_calendarView->selectionModel())
            m_calendarView->selectionModel()->clear();
        m_calendarView->setMouseTracking(true);
        m_calendarView->viewport()->setMouseTracking(true);
        m_calendarView->viewport()->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (m_calendarView && watched == m_calendarView->viewport()) {
            if (event->type() == QEvent::MouseMove) {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                const QModelIndex index = m_calendarView->indexAt(mouseEvent->position().toPoint());
                const QDate date = dateForIndex(index);
                if (date != m_hoveredDate) {
                    m_hoveredDate = date;
                    update();
                }
            } else if (event->type() == QEvent::MouseButtonPress) {
                auto *mouseEvent = static_cast<QMouseEvent *>(event);
                const QModelIndex index = m_calendarView->indexAt(mouseEvent->position().toPoint());
                const QDate date = dateForIndex(index);
                if (!date.isValid())
                    return true;
                setSelectedDate(date);
                emit clicked(date);
                return true;
            } else if (event->type() == QEvent::Leave) {
                if (m_hoveredDate.isValid()) {
                    m_hoveredDate = QDate();
                    update();
                }
            }
        }
        return QCalendarWidget::eventFilter(watched, event);
    }

    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillRect(rect, QColor(Theme::BgPrimary));

        const QDate displayDate = displayDateForCellDate(date);
        const bool isCurrentMonth = isCurrentPageDate(displayDate);
        const bool isSelected = displayDate == selectedDate();
        const bool isToday = displayDate == QDate::currentDate();
        const bool isHovered = displayDate == m_hoveredDate && !isSelected;
        const QRectF bgRect = QRectF(rect).adjusted(4, 3, -4, -3);

        if (isSelected) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(Theme::Primary));
            painter->drawRoundedRect(bgRect, 6, 6);
        } else if (isHovered && isCurrentMonth) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(Theme::PrimaryBg));
            painter->drawRoundedRect(bgRect, 6, 6);
        } else if (isToday && isCurrentMonth) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(Theme::PrimaryBg));
            painter->drawRoundedRect(bgRect, 6, 6);
        }

        QFont cellFont = painter->font();
        cellFont.setPointSize(9);
        cellFont.setBold(isSelected || (isToday && isCurrentMonth));
        painter->setFont(cellFont);

        if (isSelected) {
            painter->setPen(QColor(Theme::BgPrimary));
        } else if (!isCurrentMonth) {
            painter->setPen(QColor(Theme::TextTertiary));
            painter->setOpacity(0.42);
        } else if (isToday) {
            painter->setPen(QColor(Theme::PrimaryDark));
        } else {
            painter->setPen(QColor(Theme::TextSecondary));
        }

        painter->drawText(rect, Qt::AlignCenter, QString::number(displayDate.day()));
        painter->restore();
    }

private:
    bool isCurrentPageDate(const QDate &date) const {
        return date.isValid() && date.year() == yearShown() && date.month() == monthShown();
    }

    QDate dateForIndex(const QModelIndex &index) const {
        if (!m_calendarView || !index.isValid()) return {};

        const int firstRow = firstDateRow();
        if (index.row() < firstRow) return {};

        QDate date = firstGridDate(firstRow);
        if (!date.isValid()) return {};

        date = date.addDays((index.row() - firstRow) * 7 + index.column());
        if (shouldSkipLeadingWeek())
            date = date.addDays(7);
        return date;
    }

    QDate displayDateForCellDate(const QDate &date) const {
        return shouldSkipLeadingWeek() ? date.addDays(7) : date;
    }

    bool shouldSkipLeadingWeek() const {
        const int firstRow = firstDateRow();
        const QDate gridStart = firstGridDate(firstRow);
        if (!gridStart.isValid()) return false;
        const QDate firstOfMonth(yearShown(), monthShown(), 1);
        return gridStart.addDays(6) < firstOfMonth;
    }

    int firstDateRow() const {
        if (!m_calendarView || !m_calendarView->model()) return 0;
        for (int row = 0; row < m_calendarView->model()->rowCount(); ++row) {
            for (int col = 0; col < m_calendarView->model()->columnCount(); ++col) {
                bool ok = false;
                m_calendarView->model()->index(row, col).data(Qt::DisplayRole).toString().toInt(&ok);
                if (ok) return row;
            }
        }
        return 0;
    }

    QDate firstGridDate(int firstRow) const {
        if (!m_calendarView || !m_calendarView->model()) return {};

        const QString text = m_calendarView->model()->index(firstRow, 0).data(Qt::DisplayRole).toString();
        bool ok = false;
        const int day = text.toInt(&ok);
        if (!ok) return {};

        const QDate firstOfMonth(yearShown(), monthShown(), 1);
        if (day > 7)
            return firstOfMonth.addMonths(-1).addDays(day - 1);
        return QDate(yearShown(), monthShown(), day);
    }

    QAbstractItemView *m_calendarView = nullptr;
    QDate m_hoveredDate;
};

} // namespace

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
    m_nlpEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_nlpEdit->setStyleSheet(
        "QLineEdit { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 6px; padding: 0 8px; font-size: 13px; background: " + QString(Theme::BgPrimary) + "; }"
        "QLineEdit:focus { border-color: " + Theme::Primary + "; }");

    m_parseBtn = new QPushButton("解析");
    m_parseBtn->setFixedSize(52, 30);
    m_parseBtn->setCursor(Qt::PointingHandCursor);
    m_parseBtn->setStyleSheet(
        "QPushButton { background: " + QString(Theme::Primary) + "; color: " + Theme::BgPrimary + ";"
        "  border-radius: 6px; font-size: 13px; border: none; }"
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
    m_status->setStyleSheet("font-size: 12px; color: " + QString(Theme::TextTertiary) + ";");
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
        "  border-radius: 5px; padding: 2px 7px; font-size: 13px; }"
        "QLineEdit:focus { border-color: " + Theme::Primary + "; }";
    const QString dtSS =
        "QDateTimeEdit { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 5px; padding: 2px 6px; font-size: 13px;"
        "  background: " + Theme::BgPrimary + "; color: " + Theme::TextPrimary + "; }"
        "QDateTimeEdit:focus { border-color: " + Theme::Primary + "; }";
    const QString labelSS =
        "font-size: 13px; color: " + QString(Theme::TextSecondary) + ";";

    auto makeCalendar = []() {
        auto *cal = new ThemedCalendarWidget;
        cal->setGridVisible(false);
        cal->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
        cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

        QTextCharFormat weekdayFormat;
        weekdayFormat.setForeground(QColor(Theme::TextSecondary));
        cal->setHeaderTextFormat(weekdayFormat);
        cal->setWeekdayTextFormat(Qt::Saturday, weekdayFormat);
        cal->setWeekdayTextFormat(Qt::Sunday, weekdayFormat);

        cal->setStyleSheet(
            "QCalendarWidget {"
            "  background: " + QString(Theme::BgPrimary) + ";"
            "  color: " + Theme::TextPrimary + ";"
            "  border: 1px solid " + Theme::Border + ";"
            "}"
            "QCalendarWidget QWidget#qt_calendar_navigationbar {"
            "  background: " + QString(Theme::PrimaryDark) + ";"
            "  min-height: 36px;"
            "}"
            "QCalendarWidget QToolButton {"
            "  background: transparent;"
            "  color: " + QString(Theme::BgPrimary) + ";"
            "  border: none;"
            "  border-radius: 6px;"
            "  margin: 3px;"
            "  padding: 2px 8px;"
            "  font-size: 13px;"
            "  font-weight: 600;"
            "}"
            "QCalendarWidget QToolButton:hover {"
            "  background: " + QString(Theme::PrimaryMid) + ";"
            "}"
            "QCalendarWidget QToolButton:pressed {"
            "  background: " + QString(Theme::PrimaryMid) + ";"
            "}"
            "QCalendarWidget QToolButton:checked,"
            "QCalendarWidget QToolButton:focus {"
            "  background: transparent;"
            "  border: none;"
            "  outline: 0;"
            "}"
            "QCalendarWidget QAbstractItemView {"
            "  background: " + QString(Theme::BgPrimary) + ";"
            "  color: " + Theme::TextSecondary + ";"
            "  selection-background-color: transparent;"
            "  selection-color: " + Theme::TextSecondary + ";"
            "  alternate-background-color: " + Theme::BgSecondary + ";"
            "  border: none;"
            "  font-size: 13px;"
            "  padding-top: 8px;"
            "  outline: 0;"
            "}"
            "QCalendarWidget QAbstractItemView::item {"
            "  border: none;"
            "  outline: 0;"
            "}"
            "QCalendarWidget QAbstractItemView::item:selected,"
            "QCalendarWidget QAbstractItemView::item:focus,"
            "QCalendarWidget QAbstractItemView::item:selected:focus,"
            "QCalendarWidget QAbstractItemView::item:hover {"
            "  background: transparent;"
            "  border: none;"
            "  outline: 0;"
            "}"
        );

        auto updateCalendarTitle = [cal]() {
            if (auto *title = cal->findChild<QLabel *>("custom_calendar_title")) {
                title->setText(QString("%1 年 %2 月")
                    .arg(cal->yearShown())
                    .arg(cal->monthShown()));
            }

            cal->update();
        };

        auto setupCalendarChrome = [cal, updateCalendarTitle]() {
            if (auto *prev = cal->findChild<QToolButton *>("qt_calendar_prevmonth")) {
                prev->setIcon(QIcon());
                prev->setArrowType(Qt::NoArrow);
                prev->setText("‹");
                prev->setFixedSize(30, 28);
                prev->setFocusPolicy(Qt::NoFocus);
            }
            if (auto *next = cal->findChild<QToolButton *>("qt_calendar_nextmonth")) {
                next->setIcon(QIcon());
                next->setArrowType(Qt::NoArrow);
                next->setText("›");
                next->setFixedSize(30, 28);
                next->setFocusPolicy(Qt::NoFocus);
            }
            if (auto *month = cal->findChild<QToolButton *>("qt_calendar_monthbutton")) {
                month->hide();
            }
            if (auto *yearBtn = cal->findChild<QToolButton *>("qt_calendar_yearbutton")) {
                yearBtn->hide();
            }
            if (auto *yearEdit = cal->findChild<QWidget *>("qt_calendar_yearedit")) {
                yearEdit->hide();
            }
            if (auto *nav = cal->findChild<QWidget *>("qt_calendar_navigationbar")) {
                if (auto *layout = qobject_cast<QHBoxLayout *>(nav->layout())) {
                    auto *title = nav->findChild<QLabel *>("custom_calendar_title");
                    if (!title) {
                        title = new QLabel(nav);
                        title->setObjectName("custom_calendar_title");
                        title->setAlignment(Qt::AlignCenter);
                        title->setStyleSheet(
                            "font-size: 13px;"
                            "font-weight: 600;"
                            "color: " + QString(Theme::BgPrimary) + ";"
                            "background: transparent;");
                        layout->insertWidget(2, title, 1, Qt::AlignVCenter);
                    }
                }
            }
            updateCalendarTitle();
        };
        setupCalendarChrome();
        cal->enableHoverTracking();
        QObject::connect(cal, &QCalendarWidget::currentPageChanged, cal, updateCalendarTitle);
        return cal;
    };

    // DDL 复选框
    m_isDDLCheck = new QCheckBox("设为截止日期（DDL）");
    m_isDDLCheck->setStyleSheet("font-size: 13px; color: " + QString(Theme::TextSecondary) + ";");
    connect(m_isDDLCheck, &QCheckBox::toggled, this, &ScheduleEditPanel::onDDLToggled);
    outer->addWidget(m_isDDLCheck);

    m_form = new QFormLayout;
    auto *form = m_form;
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
    m_titleEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_titleEdit->setStyleSheet(editSS);
    form->addRow(makeLabel("标题"), m_titleEdit);

    m_startEdit = new QDateTimeEdit;
    m_startEdit->setDisplayFormat("yyyy-MM-dd  HH:mm");
    m_startEdit->setCalendarPopup(true);
    m_startEdit->setCalendarWidget(makeCalendar());
    m_startEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_startEdit->setStyleSheet(dtSS);
    form->addRow(makeLabel("开始"), m_startEdit);

    m_endEdit = new QDateTimeEdit;
    m_endEdit->setDisplayFormat("yyyy-MM-dd  HH:mm");
    m_endEdit->setCalendarPopup(true);
    m_endEdit->setCalendarWidget(makeCalendar());
    m_endEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_endEdit->setStyleSheet(dtSS);
    form->addRow(makeLabel("结束"), m_endEdit);

    m_locEdit = new QLineEdit;
    m_locEdit->setPlaceholderText("可选");
    m_locEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_locEdit->setStyleSheet(editSS);
    form->addRow(makeLabel("地点"), m_locEdit);

    m_remindBox = new QComboBox;
    m_remindBox->setStyleSheet(
        "QComboBox { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 5px; padding: 2px 6px; font-size: 13px;"
        "  color: " + QString(Theme::TextPrimary) + ";"
        "  background: " + QString(Theme::BgPrimary) + "; }"
        "QComboBox QAbstractItemView {"
        "  color: " + QString(Theme::TextPrimary) + ";"
        "  background: " + QString(Theme::BgPrimary) + ";"
        "  selection-background-color: " + QString(Theme::PrimaryBg) + "; }");
    form->addRow(makeLabel("提醒"), m_remindBox);
    onDDLToggled(false);  // 初始化为普通日程选项

    outer->addLayout(form);

    // ── 操作按钮 ────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);

    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setFixedSize(64, 28);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 6px; font-size: 13px; background: none;"
        "  color: " + Theme::TextSecondary + "; }"
        "QPushButton:hover { background: " + Theme::BgSecondary + "; }");
    connect(cancelBtn, &QPushButton::clicked, this, [this]() {
        if (m_parsing)
            m_nlp->cancelPending();
        setParsing(false);
        hide();
        emit dismissed();
    });

    m_confirmBtn = new QPushButton("确认添加");
    m_confirmBtn->setFixedHeight(28);
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setStyleSheet(
        "QPushButton { background: " + QString(Theme::Primary) + "; color: " + Theme::BgPrimary + ";"
        "  border-radius: 6px; font-size: 13px; border: none; padding: 0 12px; }"
        "QPushButton:hover { background: " + Theme::PrimaryDark + "; }");
    connect(m_confirmBtn, &QPushButton::clicked, this, &ScheduleEditPanel::onConfirm);

    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(m_confirmBtn);
    outer->addLayout(btnRow);

    adjustSize();
}

void ScheduleEditPanel::showForNew(const QDate &date) {
    m_editingId = -1;
    m_nlp->clearHistory();
    m_confirmBtn->setText("确认添加");

    m_nlpEdit->clear();
    m_nlpEdit->setPlaceholderText("用自然语言描述日程…");
    setParsing(false);
    m_status->hide();
    m_titleEdit->clear();
    m_locEdit->clear();

    // 重置 DDL 状态
    m_isDDLCheck->blockSignals(true);
    m_isDDLCheck->setChecked(false);
    m_isDDLCheck->blockSignals(false);
    onDDLToggled(false);

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

void ScheduleEditPanel::showForEdit(const Schedule &s) {
    m_editingId       = s.id;
    m_currentSchedule = s;
    m_nlp->clearHistory();
    m_confirmBtn->setText("保存修改");

    m_nlpEdit->clear();
    m_nlpEdit->setPlaceholderText("用自然语言描述，或直接编辑下方字段…");
    setParsing(false);
    m_status->hide();

    fillForm(s);

    adjustSize();
    show();
    raise();
    m_titleEdit->setFocus();
}

void ScheduleEditPanel::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 背景白底 + 边框
    QColor panelBg(Theme::BgPrimary);
    panelBg.setAlpha(250);
    p.setPen(QPen(QColor(Theme::Border), 1));
    p.setBrush(panelBg);
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
}

void ScheduleEditPanel::fillForm(const Schedule &s) {
    if (!s.title.isEmpty())    m_titleEdit->setText(s.title);
    if (s.startTime.isValid()) m_startEdit->setDateTime(s.startTime);

    // 先设 DDL 状态（会重置 remindBox 选项），再填其余字段
    m_isDDLCheck->blockSignals(true);
    m_isDDLCheck->setChecked(s.isDDL);
    m_isDDLCheck->blockSignals(false);
    onDDLToggled(s.isDDL);

    if (!s.isDDL && s.endTime.isValid()) m_endEdit->setDateTime(s.endTime);
    m_locEdit->setText(s.location);

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
        QString("font-size: 12px; color: %1;")
            .arg(isError ? "#D85A30" : Theme::TextTertiary));
    m_status->show();
    adjustSize();
    raise();
}

void ScheduleEditPanel::setParsing(bool parsing) {
    m_parsing = parsing;
    m_parseBtn->setText(parsing ? "取消" : "解析");
    m_parseBtn->setEnabled(true);
    m_nlpEdit->setEnabled(!parsing);
}

// ── DDL 切换 ──────────────────────────────────────────────────────────────────

void ScheduleEditPanel::onDDLToggled(bool on) {
    m_form->setRowVisible(m_endEdit, !on);

    // "开始" ↔ "截止时间"
    if (auto *lbl = qobject_cast<QLabel*>(m_form->labelForField(m_startEdit)))
        lbl->setText(on ? "截止" : "开始");

    m_remindBox->clear();
    if (on) {
        m_remindBox->addItem("不提醒",      0);
        m_remindBox->addItem("提前 1 天",   1440);
        m_remindBox->addItem("提前 3 天",   4320);
        m_remindBox->addItem("提前 7 天",   10080);
        m_remindBox->addItem("提前 14 天",  20160);
        m_remindBox->setCurrentIndex(1);  // 默认 1 天
    } else {
        m_remindBox->addItem("不提醒",       0);
        m_remindBox->addItem("提前 15 分钟", 15);
        m_remindBox->addItem("提前 30 分钟", 30);
        m_remindBox->addItem("提前 1 小时",  60);
        m_remindBox->addItem("提前 1 天",    1440);
        m_remindBox->setCurrentIndex(1);  // 默认 15 分钟
    }

    adjustSize();
    raise();
}

// ── NLP 槽 ─────────────────────────────────────────────────────────────────────

void ScheduleEditPanel::onParse() {
    if (m_parsing) {
        m_nlp->cancelPending();
        setParsing(false);
        setStatus("已取消解析。");
        return;
    }

    const QString text = m_nlpEdit->text().trimmed();
    if (text.isEmpty()) return;
    setParsing(true);
    setStatus("解析中…");
    if (m_editingId >= 0)
        m_nlp->parseEdit(text, m_currentSchedule);
    else
        m_nlp->parse(text);
}

void ScheduleEditPanel::onNlpParsed(const Schedule &s) {
    if (!isVisible()) return;
    setParsing(false);
    setStatus("已填入表单，请确认或修改后点击「确认添加」。");
    fillForm(s);
}

void ScheduleEditPanel::onNlpFailed(const QString &reason) {
    if (!isVisible()) return;
    setParsing(false);
    setStatus(reason, true);
}

void ScheduleEditPanel::onNlpClarification(const QString &question) {
    if (!isVisible()) return;
    setParsing(false);
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
    if (!m_isDDLCheck->isChecked() && m_endEdit->dateTime() <= m_startEdit->dateTime()) {
        setStatus("结束时间必须晚于开始时间", true);
        return;
    }

    Schedule s;
    s.id         = m_editingId;
    s.isDDL      = m_isDDLCheck->isChecked();
    s.title      = title;
    s.startTime  = m_startEdit->dateTime();
    s.endTime    = s.isDDL ? s.startTime : m_endEdit->dateTime();
    s.location   = m_locEdit->text().trimmed();
    s.remindMins = m_remindBox->currentData().toInt();

    bool ok = (m_editingId < 0) ? (m_svc->addSchedule(s) >= 0)
                                 : m_svc->updateSchedule(s);
    if (!ok) {
        const Schedule conflict = m_svc->conflictingSchedule(s);
        if (conflict.startTime.isValid()) {
            setStatus(QString("与「%1」冲突：%2 - %3，请调整时间。")
                          .arg(conflict.title,
                               conflict.startTime.toString("MM月dd日 HH:mm"),
                               conflict.endTime.toString("HH:mm")),
                      true);
        } else {
            setStatus("该时间段与已有日程冲突，请调整时间。", true);
        }
    } else {
        hide();
        emit dismissed();
    }
}
