//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/calendar/CalendarPanel.h"
#include "presentation/common/Theme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QSet>
#include <QTime>
#include <QVBoxLayout>

// ── CalendarPanel ─────────────────────────────────────────────────────────────

CalendarPanel::CalendarPanel(ScheduleService *svc, NLPService *nlp, QWidget *parent, bool embedContextPanel)
    : QWidget(parent)
    , m_svc(svc)
    , m_selDate(QDate::currentDate())
{
    setupUi(nlp, embedContextPanel);

    connect(svc, &ScheduleService::scheduleAdded,   this, [this](int){ refresh(); });
    connect(svc, &ScheduleService::scheduleUpdated, this, [this](int){ refresh(); });
    connect(svc, &ScheduleService::scheduleRemoved, this, [this](int){ refresh(); });
}

void CalendarPanel::setupUi(NLPService *nlp, bool embedContextPanel) {
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ══════════════════════════════════════════════════════════
    // 左面板
    // ══════════════════════════════════════════════════════════
    auto *left = new QWidget;
    m_contextPanel = left;
    left->setFixedWidth(320);
    left->setObjectName("calLeft");
    left->setStyleSheet("#calLeft { background: transparent; border: none; }");

    auto *leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(16, 20, 16, 20);
    leftLayout->setSpacing(0);

    // 月历
    m_miniCal = new MiniCalendar;
    leftLayout->addWidget(m_miniCal);
    connect(m_miniCal, &MiniCalendar::dateSelected,
            this,      &CalendarPanel::onDateSelected);

    // 分隔线
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: " + QString(Theme::Border) + ";");
    leftLayout->addSpacing(14);
    leftLayout->addWidget(sep);
    leftLayout->addSpacing(14);

    // "即将到来" 标题
    auto *upLabel = new QLabel("即将到来");
    upLabel->setStyleSheet(
        "font-size: 11px; font-weight: 500; color: " + QString(Theme::TextTertiary) + ";"
        "letter-spacing: 0.04em;");
    leftLayout->addWidget(upLabel);
    leftLayout->addSpacing(8);

    // 即将到来列表（动态填充）
    m_upcomingList = new QWidget;
    auto *upLayout = new QVBoxLayout(m_upcomingList);
    upLayout->setContentsMargins(0, 0, 0, 0);
    upLayout->setSpacing(0);
    leftLayout->addWidget(m_upcomingList);
    leftLayout->addStretch();

    if (embedContextPanel)
        root->addWidget(left);

    // ══════════════════════════════════════════════════════════
    // 右面板
    // ══════════════════════════════════════════════════════════
    m_rightPanel = new QWidget;
    auto *right = m_rightPanel;
    right->setObjectName("calRight");
    right->setStyleSheet("#calRight { background: transparent; }");
    auto *rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // 顶部 header
    auto *header = new QWidget;
    header->setFixedHeight(48);
    header->setObjectName("calHeader");
    header->setStyleSheet("#calHeader { background: transparent; }");

    auto *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(20, 0, 20, 0);

    m_dateLabel = new QLabel;
    m_dateLabel->setStyleSheet(
        "font-size: 15px; font-weight: 500; color: " + QString(Theme::TextPrimary) + ";"
        "border: none;");
    hLayout->addWidget(m_dateLabel);
    hLayout->addStretch();

    // 新建按钮
    auto *addBtn = new QPushButton("+ 新建");
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(
        "QPushButton { font-size: 13px; padding: 5px 14px; border-radius: 8px;"
        "  border: 1px solid " + QString(Theme::Border) + ";"
        "  background: none; color: " + Theme::TextSecondary + "; }"
        "QPushButton:hover { background: " + Theme::BgSecondary + "; }");
    connect(addBtn, &QPushButton::clicked, this, &CalendarPanel::onAddClicked);
    hLayout->addWidget(addBtn);

    rightLayout->addWidget(header);

    auto *hSep = new QFrame;
    hSep->setFrameShape(QFrame::HLine);
    hSep->setFixedHeight(1);
    hSep->setStyleSheet("background: " + QString(Theme::Border) + "; border: none;");
    rightLayout->addWidget(hSep);

    // 时间轴
    m_timeline = new TimelineView;
    rightLayout->addWidget(m_timeline, 1);

    connect(m_timeline, &TimelineView::scheduleClicked,
            this,       &CalendarPanel::onScheduleClicked);

    root->addWidget(right, 1);

    // 悬浮编辑面板（子控件，不在布局中，浮于时间轴上方）
    m_editPanel = new ScheduleEditPanel(m_svc, nlp, m_rightPanel);
    m_editPanel->hide();

    // 初始加载
    refresh();
}

void CalendarPanel::refresh() {
    // 收集所有有日程的日期，更新月历小圆点
    const QVector<Schedule> all = m_svc->getAll();
    QSet<QDate> eventDates, ddlDates;
    for (const Schedule &s : all) {
        if (s.isDDL) ddlDates.insert(s.startTime.date());
        else {
            QDate d = s.startTime.date();
            const QDate endDate = s.endTime.date();
            while (d.isValid() && d <= endDate) {
                eventDates.insert(d);
                d = d.addDays(1);
            }
        }
    }
    m_miniCal->setEventDates(eventDates);
    m_miniCal->setDdlDates(ddlDates);

    loadDay(m_selDate);
    updateUpcoming();
}

void CalendarPanel::prepareTimelineEnter() {
    if (m_timeline)
        m_timeline->prepareEnter();
}

void CalendarPanel::playTimelineEnter() {
    if (m_timeline)
        m_timeline->playEnter();
}

void CalendarPanel::playTimelineExit(const std::function<void()> &finished) {
    if (m_timeline)
        m_timeline->playExit(finished);
    else if (finished)
        finished();
}

void CalendarPanel::loadDay(const QDate &date) {
    m_selDate = date;
    m_miniCal->selectDate(date);

    // 更新右侧标题
    static const QString WD[7] = {"日","一","二","三","四","五","六"};
    const int wd = date.dayOfWeek() % 7;  // 0=Sun
    m_dateLabel->setText(
        QString("%1 年 %2 月 %3 日  周%4")
            .arg(QString::number(date.year()),
                 QString::number(date.month()),
                 QString::number(date.day()),
                 WD[wd]));

    // 过滤当天日程
    const QDateTime from(date, QTime(0, 0));
    const QDateTime to(date.addDays(1), QTime(0, 0));
    const QVector<Schedule> daySchedules = m_svc->getByDateRange(from, to);
    m_timeline->setSchedules(date, daySchedules);
}

void CalendarPanel::onDateSelected(const QDate &date) {
    loadDay(date);
}

void CalendarPanel::onAddClicked() {
    if (m_editPanel->isVisible()) {
        m_editPanel->hide();
        return;
    }
    repositionEditPanel();
    m_editPanel->showForNew(m_selDate);
}

void CalendarPanel::repositionEditPanel() {
    if (!m_editPanel || !m_rightPanel) return;
    const int x = qMax(0, m_rightPanel->width() - m_editPanel->width() - 12);
    m_editPanel->move(x, 49);  // 49 = header(48) + separator(1)
}

void CalendarPanel::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    if (m_editPanel && m_editPanel->isVisible())
        repositionEditPanel();
}

void CalendarPanel::onScheduleClicked(int id, QPoint globalPos) {
    Schedule s = m_svc->getSchedule(id);
    if (!s.startTime.isValid()) return;

    // 小悬浮框（Qt::Popup 点击外部自动关闭）
    auto *popup = new QWidget(nullptr,
        Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    popup->setAttribute(Qt::WA_TranslucentBackground);
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->setFixedWidth(240);

    auto *container = new QWidget(popup);
    container->setObjectName("schedPopup");
    container->setStyleSheet(
        "#schedPopup {"
        "  background: " + QString(Theme::BgPrimary) + ";"
        "  border-radius: 10px;"
        "  border: 1px solid " + QString(Theme::Border) + ";"
        "}");

    auto *vl = new QVBoxLayout(container);
    vl->setContentsMargins(14, 12, 14, 10);
    vl->setSpacing(4);

    auto *titleLbl = new QLabel("<b>" + s.title + "</b>", container);
    titleLbl->setStyleSheet("font-size: 13px; color: " + QString(Theme::TextPrimary) + ";");
    titleLbl->setWordWrap(true);
    vl->addWidget(titleLbl);

    const QString timeText = s.isDDL
        ? "截止：" + s.startTime.toString("MM月dd日 HH:mm")
        : (s.startTime.date() == s.endTime.date()
            ? s.startTime.toString("MM月dd日 HH:mm") + " – " + s.endTime.toString("HH:mm")
            : s.startTime.toString("MM月dd日 HH:mm") + " – " + s.endTime.toString("MM月dd日 HH:mm"));
    auto *timeLbl = new QLabel(timeText, container);
    timeLbl->setStyleSheet("font-size: 12px; color: " + QString(Theme::TextSecondary) + ";");
    vl->addWidget(timeLbl);

    if (!s.location.isEmpty()) {
        auto *locLbl = new QLabel("📍 " + s.location, container);
        locLbl->setStyleSheet("font-size: 12px; color: " + QString(Theme::TextSecondary) + ";");
        vl->addWidget(locLbl);
    }

    vl->addSpacing(6);

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(6);

    auto *deleteBtn = new QPushButton("删除", container);
    deleteBtn->setStyleSheet(
        "QPushButton { font-size: 12px; padding: 4px 10px; border-radius: 6px;"
        "  border: 1px solid #D85A30; color: #D85A30; background: none; }"
        "QPushButton:hover { background: #FAECE7; }");

    auto *editBtn = new QPushButton("编辑", container);
    editBtn->setStyleSheet(
        "QPushButton { font-size: 12px; padding: 4px 10px; border-radius: 6px;"
        "  border: 1px solid " + QString(Theme::Border) + ";"
        "  color: " + Theme::TextSecondary + "; background: none; }"
        "QPushButton:hover { background: " + Theme::BgSecondary + "; }");

    btnRow->addWidget(deleteBtn);
    btnRow->addStretch();
    btnRow->addWidget(editBtn);
    vl->addLayout(btnRow);

    auto *outerLayout = new QVBoxLayout(popup);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(container);
    popup->adjustSize();

    // 定位：在点击点附近，避免超出屏幕
    QPoint pos = globalPos - QPoint(popup->width() / 2, popup->height() + 8);
    const QRect screen = popup->screen()->availableGeometry();
    pos.setX(qBound(screen.left(), pos.x(), screen.right() - popup->width()));
    pos.setY(qBound(screen.top(), pos.y(), screen.bottom() - popup->height()));
    popup->move(pos);
    popup->show();

    QObject::connect(deleteBtn, &QPushButton::clicked, popup, [this, id, popup]() {
        popup->close();
        m_svc->removeSchedule(id);
    });

    QObject::connect(editBtn, &QPushButton::clicked, popup, [this, s, popup]() {
        popup->close();
        repositionEditPanel();
        m_editPanel->showForEdit(s);
    });
}

void CalendarPanel::updateUpcoming() {
    // 清空旧条目
    QLayout *upLayout = m_upcomingList->layout();
    while (upLayout->count()) {
        QLayoutItem *item = upLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    static const char *DOT_COLORS[4] = {
        Theme::EventPurpleBar, Theme::EventTealBar,
        Theme::EventCoralBar,  Theme::EventAmberBar
    };

    const QDateTime now = QDateTime::currentDateTime();
    QVector<Schedule> all = m_svc->getAll();

    // 排序并取最近 4 条
    std::sort(all.begin(), all.end(), [](const Schedule &a, const Schedule &b){
        return a.startTime < b.startTime;
    });

    // 普通日程取最近 6 条，DDL 取最近 7 天内 3 条，合并后按时间排序
    QVector<Schedule> upcoming;
    int regularCount = 0, ddlCount = 0;
    for (const Schedule &s : all) {
        if (s.isDDL) {
            if (ddlCount >= 3) continue;
            const int days = QDate::currentDate().daysTo(s.startTime.date());
            if (days >= 0 && days <= 7) { upcoming.append(s); ++ddlCount; }
        } else {
            if (regularCount >= 6) continue;
            if (s.startTime >= now) { upcoming.append(s); ++regularCount; }
        }
    }
    std::sort(upcoming.begin(), upcoming.end(), [](const Schedule &a, const Schedule &b){
        return a.startTime < b.startTime;
    });

    int count = 0;
    for (const Schedule &s : upcoming) {
        auto *row = new QWidget(m_upcomingList);
        auto *rl  = new QHBoxLayout(row);
        rl->setContentsMargins(0, 5, 0, 5);
        rl->setSpacing(8);

        // 彩色小圆点：DDL 固定珊瑚色
        auto *dot = new QLabel(row);
        dot->setFixedSize(6, 6);
        dot->setStyleSheet(
            QString("background: %1; border-radius: 3px;")
                .arg(s.isDDL ? Theme::EventCoralBar : DOT_COLORS[count % 4]));
        rl->addWidget(dot);

        // 标题
        auto *titleLbl = new QLabel(s.title, row);
        titleLbl->setStyleSheet(
            "font-size: 12px; color: " + QString(Theme::TextSecondary) + ";");
        titleLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        rl->addWidget(titleLbl, 1);

        // 时间：DDL 显示截止日期，普通日程显示时刻
        const QString timeStr = s.isDDL
            ? s.startTime.toString("MM/dd") + " 截止"
            : s.startTime.toString("HH:mm");
        auto *timeLbl = new QLabel(timeStr, row);
        timeLbl->setStyleSheet(
            "font-size: 11px; color: " + QString(s.isDDL ? Theme::EventCoralBar : Theme::TextTertiary) + ";");
        rl->addWidget(timeLbl);

        upLayout->addWidget(row);

        // 点击跳转到该日期并滚动到对应时刻
        row->setCursor(Qt::PointingHandCursor);
        const QDate d = s.startTime.date();
        row->setProperty("navDate", d);
        row->setProperty("navTime", s.startTime.time());
        row->installEventFilter(this);
        ++count;
    }

    if (count == 0) {
        auto *emptyLbl = new QLabel("暂无即将到来的日程", m_upcomingList);
        emptyLbl->setStyleSheet(
            "font-size: 12px; color: " + QString(Theme::TextTertiary) + ";");
        upLayout->addWidget(emptyLbl);
    }
}

bool CalendarPanel::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        const QDate d = watched->property("navDate").toDate();
        if (d.isValid()) {
            loadDay(d);
            // scrollToTime 排在 loadDay 内 singleShot(0) 之后入队，后执行覆盖默认滚动位置
            const QTime t = watched->property("navTime").toTime();
            if (t.isValid())
                m_timeline->scrollToTime(t);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
