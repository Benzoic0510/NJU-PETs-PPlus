//
// Created by Gungnir on 2026/6/1.
//

#include "presentation/other/OtherPanel.h"
#include "presentation/common/Theme.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QUrl>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QtGlobal>

namespace {

constexpr auto ProjectGithubUrl = "https://github.com/Benzoic0510/NJU-PETs-PPlus";

QString primaryButtonStyle() {
    return
        "QPushButton { background: " + QString(Theme::Primary) + ";"
        "  color: " + Theme::BgPrimary + "; border-radius: 8px;"
        "  font-size: 14px; padding: 0 14px; border: none; }"
        "QPushButton:hover { background: " + Theme::PrimaryDark + "; }";
}

QString dangerButtonStyle() {
    return
        "QPushButton { background: #D84A4A;"
        "  color: " + QString(Theme::BgPrimary) + "; border-radius: 8px;"
        "  font-size: 14px; padding: 0 14px; border: none; }"
        "QPushButton:hover { background: #B93838; }";
}

qreal clamp01(qreal value) {
    return qBound<qreal>(0.0, value, 1.0);
}

qreal easeOutCubicValue(qreal value) {
    const qreal t = 1.0 - clamp01(value);
    return 1.0 - t * t * t;
}

qreal easeInCubicValue(qreal value) {
    const qreal t = clamp01(value);
    return t * t * t;
}

qreal interpolate(qreal from, qreal to, qreal progress) {
    return from + (to - from) * clamp01(progress);
}

class OtherGroupHost : public QWidget {
public:
    explicit OtherGroupHost(QWidget *card, QWidget *parent = nullptr)
        : QWidget(parent), m_card(card)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        m_card->setParent(this);
        m_effect = new QGraphicsOpacityEffect(m_card);
        m_card->setGraphicsEffect(m_effect);
        m_effect->setOpacity(1.0);
        updateCardGeometry();
    }

    QVBoxLayout *contentLayout() const {
        return qobject_cast<QVBoxLayout *>(m_card ? m_card->layout() : nullptr);
    }

    QSize sizeHint() const override {
        if (!m_card)
            return QWidget::sizeHint();
        const QSize cardHint = m_card->sizeHint();
        return {cardHint.width(), cardHint.height() + ExtraVPad * 2};
    }

    QSize minimumSizeHint() const override {
        return sizeHint();
    }

    void prepareEnter() {
        stopAnimation();
        ++m_generation;
        setOffset(StartOffset);
        if (m_effect)
            m_effect->setOpacity(0.0);
        show();
    }

    void playEnter(int delayMs) {
        stopAnimation();
        setOffset(StartOffset);
        if (m_effect)
            m_effect->setOpacity(0.0);

        const int generation = ++m_generation;
        QTimer::singleShot(delayMs, this, [this, generation]() {
            if (generation != m_generation)
                return;

            m_anim = new QVariantAnimation(this);
            m_anim->setDuration(EnterMs);
            m_anim->setEasingCurve(QEasingCurve::Linear);
            m_anim->setStartValue(0.0);
            m_anim->setEndValue(1.0);
            connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
                const qreal t = clamp01(value.toReal());
                if (m_effect)
                    m_effect->setOpacity(easeOutCubicValue(t));

                qreal offset = 0.0;
                if (t < ForwardPart)
                    offset = interpolate(StartOffset, OvershootOffset, easeOutCubicValue(t / ForwardPart));
                else
                    offset = interpolate(OvershootOffset, 0.0,
                                         easeOutCubicValue((t - ForwardPart) / (1.0 - ForwardPart)));
                setOffset(offset);
            });
            connect(m_anim, &QVariantAnimation::finished, this, [this]() {
                setOffset(0.0);
                if (m_effect)
                    m_effect->setOpacity(1.0);
                m_anim = nullptr;
            });
            m_anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
    }

    void prepareExit() {
        stopAnimation();
        setOffset(0.0);
        if (m_effect)
            m_effect->setOpacity(1.0);
        show();
    }

    void playExit() {
        stopAnimation();
        setOffset(0.0);
        if (m_effect)
            m_effect->setOpacity(1.0);

        m_anim = new QVariantAnimation(this);
        m_anim->setDuration(ExitFadeMs);
        m_anim->setEasingCurve(QEasingCurve::Linear);
        m_anim->setStartValue(0.0);
        m_anim->setEndValue(1.0);
        connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            const qreal t = clamp01(value.toReal());
            if (m_effect)
                m_effect->setOpacity(1.0 - easeInCubicValue(t));
        });
        connect(m_anim, &QVariantAnimation::finished, this, [this]() {
            if (m_effect)
                m_effect->setOpacity(0.0);
            m_anim = nullptr;
        });
        m_anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void restoreVisible() {
        stopAnimation();
        setOffset(0.0);
        if (m_effect)
            m_effect->setOpacity(1.0);
    }

    static int delayForIndex(int index) { return index * GroupDelayMs; }
    static int exitFadeMs() { return ExitFadeMs; }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QWidget::resizeEvent(event);
        updateCardGeometry();
    }

private:
    void stopAnimation() {
        if (m_anim) {
            m_anim->stop();
            m_anim->deleteLater();
            m_anim = nullptr;
        }
    }

    void setOffset(qreal offset) {
        m_offset = offset;
        updateCardGeometry();
    }

    void updateCardGeometry() {
        if (!m_card)
            return;
        const int cardH = m_card->sizeHint().height();
        m_card->setGeometry(0, ExtraVPad + qRound(m_offset), width(), cardH);
    }

    QWidget *m_card = nullptr;
    QGraphicsOpacityEffect *m_effect = nullptr;
    QVariantAnimation *m_anim = nullptr;
    qreal m_offset = 0.0;
    int m_generation = 0;

    static constexpr int ExtraVPad = 8;
    static constexpr int GroupDelayMs = 60;
    static constexpr int EnterMs = 180;
    static constexpr int ExitFadeMs = 120;
    static constexpr qreal StartOffset = -24.0;
    static constexpr qreal OvershootOffset = 6.0;
    static constexpr qreal ForwardPart = 0.68;
};

} // namespace

OtherPanel::OtherPanel(ScheduleService *scheduleService, QWidget *parent)
    : QWidget(parent), m_scheduleService(scheduleService)
{
    setupUi();
}

void OtherPanel::setupUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background: transparent;");
    m_stack->addWidget(makeVersionPage());
    m_stack->addWidget(makeDataPage());
    m_stack->addWidget(makeHelpPage());
    root->addWidget(m_stack, 1);
}

QWidget *OtherPanel::makeGroup(const QString &title) {
    auto *group = new QWidget;
    group->setObjectName("otherGroup");
    group->setStyleSheet(
        "#otherGroup {"
        "  background:" + QString(Theme::BgPrimary) + ";"
        "  border: 1px solid " + Theme::Border + ";"
        "  border-radius: 8px;"
        "}");

    auto *root = new QVBoxLayout(group);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(12);

    auto *titleLabel = new QLabel(title, group);
    titleLabel->setStyleSheet(
        "font-size: 14px; font-weight: 700;"
        "color:" + QString(Theme::TextPrimary) + ";"
        "background: transparent; border: none;");
    root->addWidget(titleLabel);
    return new OtherGroupHost(group);
}

QWidget *OtherPanel::makeInfoRow(const QString &label, const QString &value) {
    auto *row = new QWidget;
    row->setObjectName("otherInfoRow");
    row->setStyleSheet("#otherInfoRow { background: transparent; border: none; }");

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    auto *labelWidget = new QLabel(label, row);
    labelWidget->setMinimumWidth(120);
    labelWidget->setStyleSheet(
        "font-size: 14px;"
        "color:" + QString(Theme::TextSecondary) + ";"
        "background: transparent; border: none;");
    layout->addWidget(labelWidget);

    auto *valueWidget = new QLabel(value, row);
    valueWidget->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueWidget->setWordWrap(true);
    valueWidget->setStyleSheet(
        "font-size: 14px; font-weight: 600;"
        "color:" + QString(Theme::TextPrimary) + ";"
        "background: transparent; border: none;");
    layout->addWidget(valueWidget, 1);
    return row;
}

QWidget *OtherPanel::makeLinkRow(const QString &text, const QString &url) {
    auto *row = new QWidget;
    row->setObjectName("otherLinkRow");
    row->setStyleSheet("#otherLinkRow { background: transparent; border: none; }");

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *button = new QPushButton(text, row);
    button->setCursor(Qt::PointingHandCursor);
    button->setFlat(true);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0;"
        "  color: " + QString(Theme::PrimaryDark) + ";"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  text-align: left;"
        "}"
        "QPushButton:hover {"
        "  text-decoration: underline;"
        "}");
    connect(button, &QPushButton::clicked, this, [url]() {
        if (!url.trimmed().isEmpty())
            QDesktopServices::openUrl(QUrl::fromUserInput(url));
    });

    layout->addWidget(button);
    layout->addStretch();
    return row;
}

QWidget *OtherPanel::makeActionRow(const QString &label, QWidget *control) {
    auto *row = new QWidget;
    row->setObjectName("otherActionRow");
    row->setStyleSheet("#otherActionRow { background: transparent; border: none; }");

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    auto *labelWidget = new QLabel(label, row);
    labelWidget->setMinimumWidth(148);
    labelWidget->setStyleSheet(
        "font-size: 14px;"
        "color:" + QString(Theme::TextSecondary) + ";"
        "background: transparent; border: none;");
    layout->addWidget(labelWidget);
    layout->addWidget(control, 1);
    return row;
}

QPushButton *OtherPanel::makeActionButton(const QString &text, bool primary) {
    auto *button = new QPushButton(text);
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedHeight(32);
    button->setMinimumWidth(104);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (primary) {
        button->setStyleSheet(primaryButtonStyle());
    } else {
        button->setStyleSheet(
            "QPushButton { background: transparent;"
            "  color: " + QString(Theme::TextSecondary) + ";"
            "  border: 1px solid " + Theme::Border + ";"
            "  border-radius: 8px; font-size: 14px; padding: 0 14px; }"
            "QPushButton:hover { background: " + Theme::PrimaryBg + ";"
            "  color: " + Theme::PrimaryDark + "; }"
            "QPushButton:disabled { color: " + Theme::TextTertiary + ";"
            "  background: " + Theme::BgSecondary + "; }");
    }
    return button;
}

QWidget *OtherPanel::makeVersionPage() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto *appGroup = makeGroup("应用信息");
    m_versionGroupHosts.append(appGroup);
    auto *appLayout = static_cast<OtherGroupHost *>(appGroup)->contentLayout();
    appLayout->addWidget(makeInfoRow("应用名", "NJU-PETs++"));
    appLayout->addWidget(makeInfoRow("版本号", "v0.1"));
    appLayout->addWidget(makeInfoRow("Qt 版本", QString::fromLatin1(qVersion())));

    root->addWidget(appGroup);
    root->addStretch();
    return page;
}

QWidget *OtherPanel::makeDataPage() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto *transferGroup = makeGroup("数据迁移");
    m_dataGroupHosts.append(transferGroup);
    auto *transferLayout = static_cast<OtherGroupHost *>(transferGroup)->contentLayout();

    auto *exportButton = makeActionButton("导出数据");
    exportButton->setEnabled(false);
    auto *importButton = makeActionButton("导入文件");
    importButton->setEnabled(false);
    transferLayout->addWidget(makeActionRow("导出日程数据", exportButton));
    transferLayout->addWidget(makeActionRow("导入日程数据", importButton));

    auto *cleanupGroup = makeGroup("数据清理");
    m_dataGroupHosts.append(cleanupGroup);
    auto *cleanupLayout = static_cast<OtherGroupHost *>(cleanupGroup)->contentLayout();

    m_cleanupButton = makeActionButton("清除过期", true);
    connect(m_cleanupButton, &QPushButton::clicked, this, &OtherPanel::clearExpiredSchedules);
    cleanupLayout->addWidget(makeActionRow("清除过期日程安排", m_cleanupButton));

    m_cleanupStatus = new QLabel("按日程结束时间清理，DDL 按截止时间清理。");
    m_cleanupStatus->setWordWrap(true);
    m_cleanupStatus->setStyleSheet(
        "font-size: 13px;"
        "color:" + QString(Theme::TextTertiary) + ";"
        "background: transparent; border: none;");
    cleanupLayout->addWidget(m_cleanupStatus);

    root->addWidget(transferGroup);
    root->addWidget(cleanupGroup);
    root->addStretch();
    return page;
}

QWidget *OtherPanel::makeHelpPage() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto *projectGroup = makeGroup("项目链接");
    m_helpGroupHosts.append(projectGroup);
    auto *projectLayout = static_cast<OtherGroupHost *>(projectGroup)->contentLayout();
    projectLayout->addWidget(makeLinkRow("项目 GitHub", QString::fromUtf8(ProjectGithubUrl)));

    root->addWidget(projectGroup);
    root->addStretch();
    return page;
}

QVector<QWidget *> OtherPanel::currentGroupHosts() const {
    if (!m_stack)
        return m_versionGroupHosts;
    if (m_stack->currentIndex() == 1)
        return m_dataGroupHosts;
    if (m_stack->currentIndex() == 2)
        return m_helpGroupHosts;
    return m_versionGroupHosts;
}

void OtherPanel::setCategory(int index) {
    if (!m_stack)
        return;

    const int targetIndex = qBound(0, index, m_stack->count() - 1);
    if (targetIndex == m_stack->currentIndex())
        return;

    m_stack->setCurrentIndex(targetIndex);
    prepareGroupsEnter();
    QTimer::singleShot(0, this, [this]() {
        playGroupsEnter();
    });
}

void OtherPanel::prepareGroupsEnter() {
    const QVector<QWidget *> groups = currentGroupHosts();
    for (QWidget *group : groups) {
        if (auto *host = static_cast<OtherGroupHost *>(group))
            host->prepareEnter();
    }
}

void OtherPanel::playGroupsEnter() {
    const QVector<QWidget *> groups = currentGroupHosts();
    for (int i = 0; i < groups.size(); ++i) {
        if (auto *host = static_cast<OtherGroupHost *>(groups[i]))
            host->playEnter(OtherGroupHost::delayForIndex(i));
    }
}

void OtherPanel::playGroupsExit(const std::function<void()> &finished) {
    const QVector<QWidget *> groups = currentGroupHosts();
    if (groups.isEmpty()) {
        if (finished)
            finished();
        return;
    }

    for (QWidget *group : groups) {
        if (auto *host = static_cast<OtherGroupHost *>(group)) {
            host->prepareExit();
            host->playExit();
        }
    }

    QTimer::singleShot(OtherGroupHost::exitFadeMs(), this, [this, finished]() {
        const QVector<QWidget *> groups = currentGroupHosts();
        for (QWidget *group : groups) {
            if (auto *host = static_cast<OtherGroupHost *>(group))
                host->restoreVisible();
        }
        if (finished)
            finished();
    });
}

void OtherPanel::clearExpiredSchedules() {
    if (!m_scheduleService)
        return;

    if (!m_cleanupConfirming) {
        m_cleanupConfirming = true;
        const int generation = ++m_cleanupGeneration;
        if (m_cleanupButton) {
            m_cleanupButton->setText("确认清理");
            m_cleanupButton->setStyleSheet(dangerButtonStyle());
        }
        if (m_cleanupStatus)
            m_cleanupStatus->setText("再次点击确认清理，3 秒后自动取消。");

        QTimer::singleShot(3000, this, [this, generation]() {
            if (generation != m_cleanupGeneration || !m_cleanupConfirming)
                return;

            m_cleanupConfirming = false;
            if (m_cleanupButton) {
                m_cleanupButton->setText("清除过期");
                m_cleanupButton->setStyleSheet(primaryButtonStyle());
            }
            if (m_cleanupStatus)
                m_cleanupStatus->setText("按日程结束时间清理，DDL 按截止时间清理。");
        });
        return;
    }

    m_cleanupConfirming = false;
    ++m_cleanupGeneration;
    if (m_cleanupButton) {
        m_cleanupButton->setText("清除过期");
        m_cleanupButton->setStyleSheet(primaryButtonStyle());
    }

    const QVector<Schedule> schedules = m_scheduleService->getAll();
    const QDateTime now = QDateTime::currentDateTime();
    QVector<int> expiredIds;
    for (const Schedule &schedule : schedules) {
        const QDateTime expireTime = schedule.isDDL ? schedule.startTime : schedule.endTime;
        if (expireTime.isValid() && expireTime < now)
            expiredIds.append(schedule.id);
    }

    if (expiredIds.isEmpty()) {
        if (m_cleanupStatus)
            m_cleanupStatus->setText("当前没有过期日程。");
        return;
    }

    int removed = 0;
    for (int id : expiredIds) {
        if (m_scheduleService->removeSchedule(id))
            ++removed;
    }

    if (m_cleanupStatus)
        m_cleanupStatus->setText(QString("已清除 %1 条过期日程。").arg(removed));
}
