//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/mainmenu/MainMenu.h"
#include "presentation/calendar/CalendarPanel.h"
#include "presentation/selector/PetSelector.h"
#include "presentation/settings/SettingsPanel.h"
#include "presentation/common/Theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

MainMenu::MainMenu(ScheduleService *svc, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("NJU-PETs++");
    resize(Theme::WindowW, Theme::WindowH);
    setMinimumSize(720, 480);
    setupUi(svc);
}

void MainMenu::setupUi(ScheduleService *svc) {
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Sidebar ──────────────────────────────────────────
    auto *sidebar = new QWidget;
    sidebar->setFixedWidth(Theme::SidebarWidth);
    sidebar->setObjectName("sidebar");
    sidebar->setStyleSheet(
        "QWidget#sidebar {"
        "  background:" + QString(Theme::BgSidebar) + ";"
        "  border-right: 1px solid " + Theme::Border + ";"
        "}"
    );

    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12, 24, 12, 20);
    sideLayout->setSpacing(4);

    auto *appTitle = new QLabel("NJU-PETs++");
    appTitle->setStyleSheet(
        "font-size:16px; font-weight:bold;"
        "color:" + QString(Theme::Primary) + ";"
        "padding:0 4px 20px 4px;"
    );
    sideLayout->addWidget(appTitle);

    m_navGrp = new QButtonGroup(this);
    m_navGrp->setExclusive(true);

    const QStringList labels = {"启动", "日程", "设置", "其他"};
    for (int i = 0; i < labels.size(); ++i) {
        auto *btn = makeNavBtn(labels[i]);
        m_navGrp->addButton(btn, i);
        sideLayout->addWidget(btn);
        if (i == 0) btn->setChecked(true);
    }
    sideLayout->addStretch();
    root->addWidget(sidebar);

    // ── Content ──────────────────────────────────────────
    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background:" + QString(Theme::BgPrimary) + ";");

    auto *selector = new PetSelector;
    connect(selector, &PetSelector::petSelected, this, &MainMenu::petSelected);
    m_stack->addWidget(selector);                        // 0 启动
    m_stack->addWidget(new CalendarPanel(svc));          // 1 日程
    m_stack->addWidget(new SettingsPanel);               // 2 设置
    m_stack->addWidget(new QWidget);                     // 3 其他

    root->addWidget(m_stack, 1);

    connect(m_navGrp, &QButtonGroup::idClicked,
            m_stack,  &QStackedWidget::setCurrentIndex);
}

QPushButton *MainMenu::makeNavBtn(const QString &text) {
    auto *btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setStyleSheet(
        "QPushButton {"
        "  text-align:left; padding:10px 16px;"
        "  border:none; border-radius:8px;"
        "  background:transparent;"
        "  color:" + QString(Theme::TextSecondary) + ";"
        "  font-size:14px;"
        "}"
        "QPushButton:hover {"
        "  background:" + QString(Theme::PrimaryBg) + ";"
        "  color:" + QString(Theme::Primary) + ";"
        "}"
        "QPushButton:checked {"
        "  background:" + QString(Theme::PrimaryBg) + ";"
        "  color:" + QString(Theme::Primary) + ";"
        "  font-weight:bold;"
        "}"
    );
    return btn;
}

QWidget *MainMenu::makePlaceholder(const QString &text) {
    auto *w   = new QWidget;
    auto *lbl = new QLabel(text);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet(
        "color:" + QString(Theme::TextTertiary) + ";"
        "font-size:18px;"
    );
    auto *lay = new QVBoxLayout(w);
    lay->addWidget(lbl);
    return w;
}

void MainMenu::refreshSchedules() {
    // CalendarPanel 实现后在此刷新
}
