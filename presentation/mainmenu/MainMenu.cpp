//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/mainmenu/MainMenu.h"
#include "presentation/calendar/CalendarPanel.h"
#include "presentation/selector/PetSelector.h"
#include "presentation/settings/SettingsPanel.h"
#include "presentation/common/Theme.h"
#include "data/AppConfig.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QVariantAnimation>
#include <QVBoxLayout>

namespace {

QColor mixColor(const QColor &from, const QColor &to, qreal amount) {
    const qreal t = qBound<qreal>(0.0, amount, 1.0);
    return QColor::fromRgbF(
        from.redF()   + (to.redF()   - from.redF())   * t,
        from.greenF() + (to.greenF() - from.greenF()) * t,
        from.blueF()  + (to.blueF()  - from.blueF())  * t,
        from.alphaF() + (to.alphaF() - from.alphaF()) * t
    );
}

class TopNavButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)
    Q_PROPERTY(qreal checkedProgress READ checkedProgress WRITE setCheckedProgress)

public:
    explicit TopNavButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(text, parent)
    {
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);
        setFixedHeight(34);
        setMinimumWidth(72);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        m_hoverAnim = new QPropertyAnimation(this, "hoverProgress", this);
        m_hoverAnim->setDuration(160);
        m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);

        m_checkedAnim = new QPropertyAnimation(this, "checkedProgress", this);
        m_checkedAnim->setDuration(180);
        m_checkedAnim->setEasingCurve(QEasingCurve::OutCubic);

        connect(this, &QPushButton::toggled, this, [this](bool on) {
            animate(m_checkedAnim, m_checkedProgress, on ? 1.0 : 0.0);
        });
    }

    qreal hoverProgress() const { return m_hoverProgress; }
    qreal checkedProgress() const { return m_checkedProgress; }

    void setHoverProgress(qreal progress) {
        m_hoverProgress = qBound<qreal>(0.0, progress, 1.0);
        update();
    }

    void setCheckedProgress(qreal progress) {
        m_checkedProgress = qBound<qreal>(0.0, progress, 1.0);
        update();
    }

protected:
    void enterEvent(QEnterEvent *event) override {
        QPushButton::enterEvent(event);
        animate(m_hoverAnim, m_hoverProgress, 1.0);
    }

    void leaveEvent(QEvent *event) override {
        QPushButton::leaveEvent(event);
        animate(m_hoverAnim, m_hoverProgress, 0.0);
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const QRectF bgRect = QRectF(rect()).adjusted(0.5, 1.0, -0.5, -1.0);
        QColor hoverBg(Theme::PrimaryMid);
        hoverBg.setAlphaF(0.72 * m_hoverProgress);

        if (m_hoverProgress > 0.0) {
            p.setPen(Qt::NoPen);
            p.setBrush(hoverBg);
            p.drawRoundedRect(bgRect, Theme::Radius, Theme::Radius);
        }

        if (m_checkedProgress > 0.0) {
            QColor selectedBg(Theme::BgPrimary);
            selectedBg.setAlphaF(m_checkedProgress);
            p.setPen(Qt::NoPen);
            p.setBrush(selectedBg);
            p.drawRoundedRect(bgRect, Theme::Radius, Theme::Radius);
        }

        QFont textFont = font();
        textFont.setPointSize(10);
        textFont.setBold(m_checkedProgress > 0.5);
        p.setFont(textFont);
        p.setPen(mixColor(QColor(Theme::BgPrimary), QColor(Theme::PrimaryDark), m_checkedProgress));
        p.drawText(rect(), Qt::AlignCenter, text());
    }

private:
    void animate(QPropertyAnimation *anim, qreal from, qreal to) {
        anim->stop();
        anim->setStartValue(from);
        anim->setEndValue(to);
        anim->start();
    }

    qreal m_hoverProgress = 0.0;
    qreal m_checkedProgress = 0.0;
    QPropertyAnimation *m_hoverAnim = nullptr;
    QPropertyAnimation *m_checkedAnim = nullptr;
};

class TopWindowButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)

public:
    explicit TopWindowButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(text, parent)
    {
        setFixedSize(34, 30);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);

        m_hoverAnim = new QPropertyAnimation(this, "hoverProgress", this);
        m_hoverAnim->setDuration(150);
        m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);
    }

    qreal hoverProgress() const { return m_hoverProgress; }

    void setHoverProgress(qreal progress) {
        m_hoverProgress = qBound<qreal>(0.0, progress, 1.0);
        update();
    }

protected:
    void enterEvent(QEnterEvent *event) override {
        QPushButton::enterEvent(event);
        animate(1.0);
    }

    void leaveEvent(QEvent *event) override {
        QPushButton::leaveEvent(event);
        animate(0.0);
    }

    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QColor bg(Theme::PrimaryMid);
        bg.setAlphaF(0.82 * m_hoverProgress);
        if (m_hoverProgress > 0.0) {
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 6, 6);
        }

        QFont textFont = font();
        textFont.setPointSize(11);
        textFont.setBold(true);
        p.setFont(textFont);
        p.setPen(QColor(Theme::BgPrimary));
        p.drawText(rect(), Qt::AlignCenter, text());
    }

private:
    void animate(qreal to) {
        m_hoverAnim->stop();
        m_hoverAnim->setStartValue(m_hoverProgress);
        m_hoverAnim->setEndValue(to);
        m_hoverAnim->start();
    }

    qreal m_hoverProgress = 0.0;
    QPropertyAnimation *m_hoverAnim = nullptr;
};

} // namespace

MainMenu::MainMenu(ScheduleService *svc, NLPService *nlp, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("NJU-PETs++");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    resize(Theme::WindowW, Theme::WindowH);
    setMinimumSize(720, 480);
    setupUi(svc, nlp);
}

void MainMenu::setupUi(ScheduleService *svc, NLPService *nlp) {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Custom title / navigation bar ───────────────────
    m_titleBar = new QWidget;
    m_titleBar->setFixedHeight(56);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setStyleSheet(
        "QWidget#titleBar {"
        "  background:" + QString(Theme::PrimaryDark) + ";"
        "  border-bottom: 1px solid " + Theme::Primary + ";"
        "}"
    );
    m_titleBar->setProperty("windowDragArea", true);
    m_titleBar->installEventFilter(this);

    auto *barLayout = new QHBoxLayout(m_titleBar);
    barLayout->setContentsMargins(16, 0, 12, 0);
    barLayout->setSpacing(0);

    auto *appTitle = new QLabel("NJU-PETs++");
    appTitle->setFixedWidth(176);
    appTitle->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    appTitle->setStyleSheet(
        "font-size:16px; font-weight:700;"
        "color:" + QString(Theme::BgPrimary) + ";"
        "letter-spacing:0;"
    );
    appTitle->setProperty("windowDragArea", true);
    appTitle->installEventFilter(this);
    barLayout->addWidget(appTitle);

    auto *navWrap = new QWidget;
    navWrap->setProperty("windowDragArea", true);
    navWrap->installEventFilter(this);
    auto *navLayout = new QHBoxLayout(navWrap);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(8);
    navLayout->addStretch();

    m_navGrp = new QButtonGroup(this);
    m_navGrp->setExclusive(true);

    const QStringList labels = {"启动", "日程", "设置", "其他"};
    for (int i = 0; i < labels.size(); ++i) {
        auto *btn = makeNavBtn(labels[i]);
        m_navGrp->addButton(btn, i);
        navLayout->addWidget(btn);
        if (i == 0) btn->setChecked(true);
    }
    navLayout->addStretch();
    barLayout->addWidget(navWrap, 1);

    auto *windowControls = new QWidget;
    windowControls->setFixedWidth(176);
    windowControls->setProperty("windowDragArea", true);
    windowControls->installEventFilter(this);
    auto *controlLayout = new QHBoxLayout(windowControls);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(6);
    controlLayout->addStretch();

    auto *minBtn = new TopWindowButton("−");
    auto *closeBtn = new TopWindowButton("×");
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    controlLayout->addWidget(minBtn);
    controlLayout->addWidget(closeBtn);
    barLayout->addWidget(windowControls);

    root->addWidget(m_titleBar);

    // ── Content shell ───────────────────────────────────
    auto *contentShell = new QWidget;
    contentShell->setObjectName("contentShell");
    contentShell->setStyleSheet(
        "QWidget#contentShell { background:" + QString(Theme::PrimaryBg) + "; }");
    auto *contentLayout = new QHBoxLayout(contentShell);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_contextPanel = new QWidget;
    m_contextPanel->setObjectName("contextPanel");
    m_contextPanel->setFixedWidth(260);
    m_contextPanel->setStyleSheet(
        "QWidget#contextPanel {"
        "  background:" + QString(Theme::BgPrimary) + ";"
        "  border-right: 1px solid " + Theme::Border + ";"
        "}");
    auto *contextLayout = new QVBoxLayout(m_contextPanel);
    contextLayout->setContentsMargins(0, 0, 0, 0);
    contextLayout->setSpacing(0);

    m_contextStack = new QStackedWidget;
    m_contextStack->setStyleSheet("background: transparent;");
    contextLayout->addWidget(m_contextStack);
    contentLayout->addWidget(m_contextPanel);

    auto *rightWrap = new QWidget;
    rightWrap->setObjectName("rightContentWrap");
    rightWrap->setStyleSheet("#rightContentWrap { background: transparent; }");
    auto *rightWrapLayout = new QVBoxLayout(rightWrap);
    rightWrapLayout->setContentsMargins(18, 18, 18, 18);
    rightWrapLayout->setSpacing(0);

    auto *rightSurface = new QWidget;
    rightSurface->setObjectName("rightContentSurface");
    rightSurface->setStyleSheet(
        "#rightContentSurface {"
        "  background:" + QString(Theme::BgPrimary) + ";"
        "  border: 1px solid " + Theme::Border + ";"
        "  border-radius: 8px;"
        "}");
    auto *surfaceLayout = new QVBoxLayout(rightSurface);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    surfaceLayout->setSpacing(0);

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background: transparent;");

    auto *selector = new PetSelector;
    selector->setStyleSheet("background: transparent;");
    connect(selector, &PetSelector::petSelected, this, [this](const QString &petId) {
        updatePetContext(petId);
        emit petSelected(petId);
    });

    auto *calendarPanel = new CalendarPanel(svc, nlp, nullptr, false);
    calendarPanel->setStyleSheet("background: transparent;");
    auto *settingsPanel = new SettingsPanel;
    settingsPanel->setStyleSheet("background: transparent;");
    connect(settingsPanel, &SettingsPanel::petScaleChanged,
            this,          &MainMenu::petScaleChanged);

    m_contextStack->addWidget(makePetContext());                 // 0 启动
    m_contextStack->addWidget(calendarPanel->contextPanel());    // 1 日程
    m_contextStack->addWidget(makeSettingsContext(settingsPanel));// 2 设置
    m_contextStack->addWidget(new QWidget);                      // 3 其他

    m_stack->addWidget(selector);                                // 0 启动
    m_stack->addWidget(calendarPanel);                           // 1 日程
    m_stack->addWidget(settingsPanel);                           // 2 设置
    m_stack->addWidget(new QWidget);                             // 3 其他

    surfaceLayout->addWidget(m_stack);
    rightWrapLayout->addWidget(rightSurface);
    contentLayout->addWidget(rightWrap, 1);
    root->addWidget(contentShell, 1);

    connect(m_navGrp, &QButtonGroup::idClicked, this, &MainMenu::switchPage);
}

QPushButton *MainMenu::makeNavBtn(const QString &text) {
    return new TopNavButton(text);
}

QPushButton *MainMenu::makeContextBtn(const QString &text) {
    auto *btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setFixedHeight(34);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setStyleSheet(
        "QPushButton {"
        "  text-align:left; padding:0 12px;"
        "  border:none; border-radius:8px;"
        "  background:transparent;"
        "  color:" + QString(Theme::TextSecondary) + ";"
        "  font-size:13px;"
        "}"
        "QPushButton:hover {"
        "  background:" + QString(Theme::PrimaryBg) + ";"
        "  color:" + QString(Theme::PrimaryDark) + ";"
        "}"
        "QPushButton:checked {"
        "  background:" + QString(Theme::PrimaryBg) + ";"
        "  color:" + QString(Theme::PrimaryDark) + ";"
        "  font-weight:600;"
        "}"
    );
    return btn;
}

QWidget *MainMenu::makePetContext() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 22, 18, 18);
    root->setSpacing(14);

    auto *label = new QLabel("当前宠物");
    label->setStyleSheet(
        "font-size: 12px; font-weight: 600;"
        "color:" + QString(Theme::TextTertiary) + ";"
        "letter-spacing:0;");
    root->addWidget(label);

    m_petPreview = new QLabel;
    m_petPreview->setFixedSize(180, 180);
    m_petPreview->setAlignment(Qt::AlignCenter);
    m_petPreview->setStyleSheet(
        "background:" + QString(Theme::PrimaryBg) + ";"
        "border-radius: 8px;");
    root->addWidget(m_petPreview, 0, Qt::AlignHCenter);

    m_petName = new QLabel;
    m_petName->setAlignment(Qt::AlignCenter);
    m_petName->setStyleSheet(
        "font-size: 15px; font-weight: 600;"
        "color:" + QString(Theme::TextPrimary) + ";");
    root->addWidget(m_petName);

    auto *hint = new QLabel("在右侧选择卡片即可切换桌宠形象。");
    hint->setWordWrap(true);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet(
        "font-size: 12px;"
        "color:" + QString(Theme::TextTertiary) + ";");
    root->addWidget(hint);
    root->addStretch();

    updatePetContext(AppConfig::instance().petId());
    return page;
}

QWidget *MainMenu::makeSettingsContext(SettingsPanel *settingsPanel) {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(14, 22, 14, 18);
    root->setSpacing(10);

    auto *label = new QLabel("设置类别");
    label->setStyleSheet(
        "font-size: 12px; font-weight: 600;"
        "color:" + QString(Theme::TextTertiary) + ";"
        "letter-spacing:0;");
    root->addWidget(label);

    auto *group = new QButtonGroup(page);
    group->setExclusive(true);

    const QStringList labels = {"宠物", "日程"};
    for (int i = 0; i < labels.size(); ++i) {
        auto *btn = makeContextBtn(labels[i]);
        group->addButton(btn, i);
        root->addWidget(btn);
        if (i == 0) btn->setChecked(true);
    }

    connect(group, &QButtonGroup::idClicked, settingsPanel, &SettingsPanel::setCategory);
    root->addStretch();
    return page;
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

void MainMenu::switchPage(int id) {
    if (!m_stack || !m_contextStack) return;

    m_stack->setCurrentIndex(id);
    m_contextStack->setCurrentIndex(id);

    static constexpr int ContextWidths[] = {260, 260, 196, 0};
    const int index = qBound(0, id, 3);
    animateContextWidth(ContextWidths[index]);
}

void MainMenu::animateContextWidth(int targetWidth) {
    if (!m_contextPanel) return;

    if (!m_contextWidthAnim) {
        m_contextWidthAnim = new QVariantAnimation(this);
        m_contextWidthAnim->setDuration(260);
        m_contextWidthAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_contextWidthAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            m_contextPanel->setFixedWidth(value.toInt());
        });
    }

    if (targetWidth > 0)
        m_contextPanel->show();

    m_contextWidthAnim->stop();
    m_contextWidthAnim->setStartValue(m_contextPanel->width());
    m_contextWidthAnim->setEndValue(targetWidth);
    m_contextWidthAnim->start();
}

void MainMenu::updatePetContext(const QString &petId) {
    if (!m_petPreview || !m_petName) return;

    m_petName->setText(petId);
    QPixmap sheet(QString(":/sprites/%1_idle.png").arg(petId));
    if (!sheet.isNull()) {
        m_petPreview->setPixmap(sheet.copy(0, 0, 128, 128).scaled(
            152, 152, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_petPreview->setText({});
    } else {
        m_petPreview->setPixmap({});
        m_petPreview->setText("NJU-PETs++");
    }
}

bool MainMenu::eventFilter(QObject *watched, QEvent *event) {
    if (watched->property("windowDragArea").toBool()) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragStartGlobal = mouseEvent->globalPosition().toPoint();
                m_windowStartPos = frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && m_dragging) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            move(m_windowStartPos + mouseEvent->globalPosition().toPoint() - m_dragStartGlobal);
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

#include "MainMenu.moc"
