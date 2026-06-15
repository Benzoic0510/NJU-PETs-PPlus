//
// Created by BenzoicAcid on 2026/5/15.
//

#include "presentation/settings/SettingsPanel.h"
#include "data/AppConfig.h"
#include "presentation/common/Theme.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <QUrl>
#include <QVariantAnimation>
#include <QVBoxLayout>

namespace {

constexpr int DefaultPetScale = 100;
constexpr int DefaultSleepThresholdMins = 5;
constexpr bool DefaultInteractionAllowed = true;
constexpr bool DefaultReminderEnabled = true;

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

class SettingsGroupHost : public QWidget {
public:
    explicit SettingsGroupHost(QWidget *card, QWidget *parent = nullptr)
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

SettingsPanel::SettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SettingsPanel::setupUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background: transparent;");
    m_stack->addWidget(makePetPage());
    m_stack->addWidget(makeSchedulePage());
    m_stack->addWidget(makeSoundPage());
    root->addWidget(m_stack, 1);
}

QWidget *SettingsPanel::makeGroup(const QString &title) {
    auto *group = new QWidget;
    group->setObjectName("settingsGroup");
    group->setStyleSheet(
        "#settingsGroup {"
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
    return new SettingsGroupHost(group);
}

QWidget *SettingsPanel::makeSettingRow(const QString &label, QWidget *control, QPushButton *resetButton) {
    auto *row = new QWidget;
    row->setObjectName("settingRow");
    row->setStyleSheet("#settingRow { background: transparent; border: none; }");

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(12);

    auto *labelWidget = new QLabel(label, row);
    labelWidget->setMinimumWidth(148);
    labelWidget->setStyleSheet(
        "font-size: 14px;"
        "color:" + QString(Theme::TextSecondary) + ";"
        "background: transparent; border: none;");
    layout->addWidget(labelWidget, 0, Qt::AlignVCenter);
    layout->addWidget(control, 1, Qt::AlignVCenter);

    if (resetButton) {
        layout->addWidget(resetButton, 0, Qt::AlignRight | Qt::AlignVCenter);
    } else {
        auto *spacer = new QWidget(row);
        spacer->setFixedSize(26, 26);
        spacer->setVisible(false);
        layout->addWidget(spacer, 0, Qt::AlignRight | Qt::AlignVCenter);
    }
    return row;
}

QPushButton *SettingsPanel::makeResetButton() {
    auto *button = new QPushButton(QString::fromUtf8("↻"));
    button->setFixedSize(26, 26);
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip("重置为默认值");
    button->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 13px;"
        "  color: " + QString(Theme::TextTertiary) + ";"
        "  font-size: 14px;"
        "  font-weight: 700;"
        "  padding: 0;"
        "}"
        "QPushButton:hover {"
        "  background: " + QString(Theme::PrimaryBg) + ";"
        "  border-color: " + Theme::PrimaryMid + ";"
        "  color: " + Theme::PrimaryDark + ";"
        "}"
        "QPushButton:disabled {"
        "  background: transparent;"
        "  border-color: transparent;"
        "  color: transparent;"
        "}");
    button->setEnabled(false);
    return button;
}

QWidget *SettingsPanel::makePetPage() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto *paramsGroup = makeGroup("宠物参数");
    m_petGroupHosts.append(paramsGroup);
    auto *paramsLayout = static_cast<SettingsGroupHost *>(paramsGroup)->contentLayout();

    auto *scaleWidget = new QWidget;
    auto *scaleRow = new QHBoxLayout(scaleWidget);
    scaleRow->setContentsMargins(0, 0, 0, 0);
    scaleRow->setSpacing(8);

    m_petScaleSlider = new QSlider(Qt::Horizontal);
    m_petScaleSlider->setRange(60, 180);
    m_petScaleSlider->setValue(AppConfig::instance().petScale());

    m_petScaleLabel = new QLabel(QString("%1%").arg(AppConfig::instance().petScale()));
    m_petScaleLabel->setFixedWidth(44);
    m_petScaleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_petScaleLabel->setStyleSheet("font-size: 13px; color: " + QString(Theme::TextSecondary) + ";");

    connect(m_petScaleSlider, &QSlider::valueChanged, this, [this](int v) {
        m_petScaleLabel->setText(QString("%1%").arg(v));
        AppConfig::instance().setPetScale(v);
        AppConfig::instance().save();
        updateResetButtons();
        emit petScaleChanged(v);
    });

    scaleRow->addWidget(m_petScaleSlider, 1);
    scaleRow->addWidget(m_petScaleLabel);
    m_petScaleResetBtn = makeResetButton();
    connect(m_petScaleResetBtn, &QPushButton::clicked, this, [this]() {
        m_petScaleSlider->setValue(DefaultPetScale);
    });
    paramsLayout->addWidget(makeSettingRow("宠物大小", scaleWidget, m_petScaleResetBtn));

    auto *sleepWidget = new QWidget;
    auto *sleepRow = new QHBoxLayout(sleepWidget);
    sleepRow->setContentsMargins(0, 0, 0, 0);
    sleepRow->setSpacing(8);

    m_sleepThresholdSlider = new QSlider(Qt::Horizontal);
    m_sleepThresholdSlider->setRange(1, 60);
    m_sleepThresholdSlider->setValue(AppConfig::instance().petSleepThresholdMins());

    m_sleepThresholdLabel = new QLabel(QString("%1 分钟").arg(AppConfig::instance().petSleepThresholdMins()));
    m_sleepThresholdLabel->setFixedWidth(58);
    m_sleepThresholdLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_sleepThresholdLabel->setStyleSheet("font-size: 13px; color: " + QString(Theme::TextSecondary) + ";");

    connect(m_sleepThresholdSlider, &QSlider::valueChanged, this, [this](int v) {
        m_sleepThresholdLabel->setText(QString("%1 分钟").arg(v));
        AppConfig::instance().setPetSleepThresholdMins(v);
        AppConfig::instance().save();
        updateResetButtons();
        emit petSleepThresholdChanged(v);
    });

    sleepRow->addWidget(m_sleepThresholdSlider, 1);
    sleepRow->addWidget(m_sleepThresholdLabel);
    m_sleepThresholdResetBtn = makeResetButton();
    connect(m_sleepThresholdResetBtn, &QPushButton::clicked, this, [this]() {
        m_sleepThresholdSlider->setValue(DefaultSleepThresholdMins);
    });
    paramsLayout->addWidget(makeSettingRow("睡眠状态时间阈值", sleepWidget, m_sleepThresholdResetBtn));

    auto *interactionGroup = makeGroup("交互");
    m_petGroupHosts.append(interactionGroup);
    auto *interactionLayout = static_cast<SettingsGroupHost *>(interactionGroup)->contentLayout();

    m_interactionDisabledCheck = new QCheckBox("允许与宠物交互");
    m_interactionDisabledCheck->setChecked(!AppConfig::instance().petInteractionDisabled());
    m_interactionDisabledCheck->setStyleSheet("font-size: 14px; color: " + QString(Theme::TextSecondary) + ";");
    connect(m_interactionDisabledCheck, &QCheckBox::toggled, this, [this](bool checked) {
        AppConfig::instance().setPetInteractionDisabled(!checked);
        AppConfig::instance().save();
        updateResetButtons();
        emit petInteractionDisabledChanged(!checked);
    });
    m_interactionResetBtn = makeResetButton();
    connect(m_interactionResetBtn, &QPushButton::clicked, this, [this]() {
        m_interactionDisabledCheck->setChecked(DefaultInteractionAllowed);
    });
    interactionLayout->addWidget(makeSettingRow("宠物交互", m_interactionDisabledCheck, m_interactionResetBtn));

    root->addWidget(paramsGroup);
    root->addWidget(interactionGroup);
    root->addStretch();
    updateResetButtons();
    return page;
}

QWidget *SettingsPanel::makeSchedulePage() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    const bool envSet = !qgetenv("DASHSCOPE_API_KEY").isEmpty();

    auto *apiGroup = makeGroup("API Key");
    m_scheduleGroupHosts.append(apiGroup);
    auto *apiLayout = static_cast<SettingsGroupHost *>(apiGroup)->contentLayout();

    auto *keyWidget = new QWidget;
    auto *keyVl = new QVBoxLayout(keyWidget);
    keyVl->setContentsMargins(0, 0, 0, 0);
    keyVl->setSpacing(4);

    auto *keyRow = new QHBoxLayout;
    static constexpr int KeyRowH = 32;

    m_apiKeyEdit = new QLineEdit;
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("sk-...");
    m_apiKeyEdit->setText(AppConfig::instance().apiKey());
    m_apiKeyEdit->setEnabled(!envSet);
    m_apiKeyEdit->setFixedHeight(KeyRowH);

    auto *toggleBtn = new QPushButton("显示");
    toggleBtn->setFixedSize(48, KeyRowH);
    toggleBtn->setStyleSheet(
        "QPushButton { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 6px; font-size: 13px; background: none;"
        "  color: " + Theme::TextSecondary + "; }"
        "QPushButton:hover { background: " + Theme::BgSecondary + "; }");
    connect(toggleBtn, &QPushButton::toggled, this, [this, toggleBtn](bool on) {
        m_apiKeyEdit->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
        toggleBtn->setText(on ? "隐藏" : "显示");
    });
    toggleBtn->setCheckable(true);

    keyRow->addWidget(m_apiKeyEdit, 1);
    keyRow->addWidget(toggleBtn);
    keyVl->addLayout(keyRow);

    if (envSet) {
        auto *envHint = new QLabel("已检测到环境变量 DASHSCOPE_API_KEY，运行时优先使用，此处不可编辑。");
        envHint->setStyleSheet("font-size: 12px; color: " + QString(Theme::TextTertiary) + ";");
        envHint->setWordWrap(true);
        keyVl->addWidget(envHint);
    }
    apiLayout->addWidget(makeSettingRow("设置 API Key", keyWidget));

    m_saveBtn = new QPushButton("保存设置");
    m_saveBtn->setFixedWidth(100);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(
        "QPushButton { background: " + QString(Theme::Primary) + "; color: " + Theme::BgPrimary + ";"
        "  border-radius: 8px; font-size: 14px; padding: 7px 0; border: none; }"
        "QPushButton:hover { background: " + Theme::PrimaryDark + "; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsPanel::onSave);
    apiLayout->addWidget(m_saveBtn, 0, Qt::AlignLeft);

    auto *reminderGroup = makeGroup("日程提醒");
    m_scheduleGroupHosts.append(reminderGroup);
    auto *reminderLayout = static_cast<SettingsGroupHost *>(reminderGroup)->contentLayout();

    m_reminderCheck = new QCheckBox("启用日程提醒");
    m_reminderCheck->setChecked(AppConfig::instance().reminderEnabled());
    m_reminderCheck->setStyleSheet("font-size: 14px; color: " + QString(Theme::TextSecondary) + ";");
    connect(m_reminderCheck, &QCheckBox::toggled, this, [this](bool checked) {
        AppConfig::instance().setReminderEnabled(checked);
        AppConfig::instance().save();
        updateResetButtons();
    });
    m_reminderResetBtn = makeResetButton();
    connect(m_reminderResetBtn, &QPushButton::clicked, this, [this]() {
        m_reminderCheck->setChecked(DefaultReminderEnabled);
    });
    reminderLayout->addWidget(makeSettingRow("是否启用日程提醒", m_reminderCheck, m_reminderResetBtn));

    root->addWidget(apiGroup);
    root->addWidget(reminderGroup);
    root->addStretch();
    updateResetButtons();
    return page;
}

void SettingsPanel::setCategory(int index) {
    if (!m_stack) return;
    const int targetIndex = qBound(0, index, m_stack->count() - 1);
    if (targetIndex == m_stack->currentIndex())
        return;

    m_stack->setCurrentIndex(targetIndex);
    prepareGroupsEnter();
    QTimer::singleShot(0, this, [this]() {
        playGroupsEnter();
    });
}

QVector<QWidget *> SettingsPanel::currentGroupHosts() const {
    if (!m_stack)
        return {};
    if (m_stack->currentIndex() == 0)
        return m_petGroupHosts;
    if (m_stack->currentIndex() == 1)
        return m_scheduleGroupHosts;
    return m_soundGroupHosts;
}

void SettingsPanel::prepareGroupsEnter() {
    const QVector<QWidget *> groups = currentGroupHosts();
    for (QWidget *group : groups) {
        if (auto *host = static_cast<SettingsGroupHost *>(group))
            host->prepareEnter();
    }
}

void SettingsPanel::playGroupsEnter() {
    const QVector<QWidget *> groups = currentGroupHosts();
    for (int i = 0; i < groups.size(); ++i) {
        if (auto *host = static_cast<SettingsGroupHost *>(groups[i]))
            host->playEnter(SettingsGroupHost::delayForIndex(i));
    }
}

void SettingsPanel::playGroupsExit(const std::function<void()> &finished) {
    const QVector<QWidget *> groups = currentGroupHosts();
    if (groups.isEmpty()) {
        if (finished)
            finished();
        return;
    }

    for (QWidget *group : groups) {
        if (auto *host = static_cast<SettingsGroupHost *>(group)) {
            host->prepareExit();
            host->playExit();
        }
    }

    QTimer::singleShot(SettingsGroupHost::exitFadeMs(), this, [this, finished]() {
        const QVector<QWidget *> groups = currentGroupHosts();
        for (QWidget *group : groups) {
            if (auto *host = static_cast<SettingsGroupHost *>(group))
                host->restoreVisible();
        }
        if (finished)
            finished();
    });
}

void SettingsPanel::updateResetButtons() {
    if (m_petScaleResetBtn && m_petScaleSlider)
        m_petScaleResetBtn->setEnabled(m_petScaleSlider->value() != DefaultPetScale);

    if (m_sleepThresholdResetBtn && m_sleepThresholdSlider)
        m_sleepThresholdResetBtn->setEnabled(m_sleepThresholdSlider->value() != DefaultSleepThresholdMins);

    if (m_interactionResetBtn && m_interactionDisabledCheck)
        m_interactionResetBtn->setEnabled(m_interactionDisabledCheck->isChecked() != DefaultInteractionAllowed);

    if (m_reminderResetBtn && m_reminderCheck)
        m_reminderResetBtn->setEnabled(m_reminderCheck->isChecked() != DefaultReminderEnabled);
}

void SettingsPanel::onSave() {
    AppConfig &cfg = AppConfig::instance();
    if (qgetenv("DASHSCOPE_API_KEY").isEmpty())
        cfg.setApiKey(m_apiKeyEdit->text().trimmed());
    cfg.save();
    updateResetButtons();

    m_saveBtn->setText("保存成功");
    m_saveBtn->setEnabled(false);
    QTimer::singleShot(2000, this, [this]() {
        m_saveBtn->setText("保存设置");
        m_saveBtn->setEnabled(true);
    });
}

// ═══════════════════════════════════════════════════════════
// 音效页面
// ═══════════════════════════════════════════════════════════

QWidget *SettingsPanel::makeSoundPage() {
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto *soundGroup = makeGroup("状态 / 操作音效");
    m_soundGroupHosts.append(soundGroup);
    auto *soundLayout = static_cast<SettingsGroupHost *>(soundGroup)->contentLayout();

    auto *previewPlayer = new QMediaPlayer(page);
    auto *previewAudio  = new QAudioOutput(page);
    previewAudio->setVolume(1.0);
    previewPlayer->setAudioOutput(previewAudio);
    connect(previewPlayer, &QMediaPlayer::errorOccurred, page,
            [](QMediaPlayer::Error, const QString &errorString) {
                qWarning() << "Preview sound failed:" << errorString;
            });

    struct SoundEvent {
        QString key;
        QString label;
    };
    const QVector<SoundEvent> events = {
        {"greet",      QString::fromUtf8("互动 / 点击")},
        {"remind",     QString::fromUtf8("日程提醒")},
        {"idle",       QString::fromUtf8("待机")},
        {"sleep",      QString::fromUtf8("入睡")},
        {"wake",       QString::fromUtf8("醒来")},
        {"drag_start", QString::fromUtf8("拖拽开始")},
        {"drag_end",   QString::fromUtf8("拖拽结束")},
    };

    for (const auto &ev : events)
        soundLayout->addWidget(makeSoundRow(ev.label, ev.key, previewPlayer));

    root->addWidget(soundGroup);
    root->addStretch();
    return page;
}

QWidget *SettingsPanel::makeSoundRow(const QString &label, const QString &eventKey,
                                     QMediaPlayer *previewPlayer) {
    auto *row = new QWidget;
    row->setObjectName("soundRow");
    row->setStyleSheet("#soundRow { background: transparent; border: none; }");

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 4, 0, 4);
    layout->setSpacing(8);

    // 事件名称
    auto *labelWidget = new QLabel(label, row);
    labelWidget->setFixedWidth(72);
    labelWidget->setStyleSheet(
        "font-size: 14px; font-weight: 600;"
        "color:" + QString(Theme::TextPrimary) + ";"
        "background: transparent; border: none;");
    layout->addWidget(labelWidget);

    // 文件名
    auto *fileLabel = new QLabel(row);
    fileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    fileLabel->setStyleSheet(
        "font-size: 13px;"
        "color:" + QString(Theme::TextTertiary) + ";"
        "background: transparent; border: none;");

    auto updateFileLabel = [fileLabel, eventKey]() {
        const QString path = AppConfig::instance().soundForEvent(eventKey);
        if (path.isEmpty()) {
            fileLabel->setText(QString::fromUtf8("未设置"));
        } else {
            const QFileInfo fi(path);
            fileLabel->setText(fi.exists() ? fi.fileName()
                                           : QString::fromUtf8("(文件不存在)"));
        }
    };
    updateFileLabel();
    layout->addWidget(fileLabel, 1);

    const QString btnStyle =
        "QPushButton {"
        "  background: transparent;"
        "  border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 6px;"
        "  color: " + Theme::TextSecondary + ";"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background: " + Theme::PrimaryBg + ";"
        "  border-color: " + Theme::PrimaryMid + ";"
        "  color: " + Theme::PrimaryDark + ";"
        "}";

    // 导入
    auto *importBtn = new QPushButton(QString::fromUtf8("导入"), row);
    importBtn->setFixedSize(48, 28);
    importBtn->setCursor(Qt::PointingHandCursor);
    importBtn->setFocusPolicy(Qt::NoFocus);
    importBtn->setStyleSheet(btnStyle);
    connect(importBtn, &QPushButton::clicked, this, [this, eventKey, updateFileLabel]() {
        const QString file = QFileDialog::getOpenFileName(
            this, QString::fromUtf8("选择音效文件"), QString(),
            QString::fromUtf8("音频文件 (*.mp3 *.wav *.ogg *.flac *.aac);;所有文件 (*)"));
        if (!file.isEmpty()) {
            AppConfig::instance().setSoundForEvent(eventKey, file);
            AppConfig::instance().save();
            updateFileLabel();
        }
    });
    layout->addWidget(importBtn);

    // 试听
    auto *previewBtn = new QPushButton(QString::fromUtf8("试听"), row);
    previewBtn->setFixedSize(48, 28);
    previewBtn->setCursor(Qt::PointingHandCursor);
    previewBtn->setFocusPolicy(Qt::NoFocus);
    previewBtn->setStyleSheet(btnStyle);
    connect(previewBtn, &QPushButton::clicked, this, [previewPlayer, eventKey]() {
        const QString path = AppConfig::instance().soundForEvent(eventKey);
        if (path.isEmpty())
            return;

        const QFileInfo fi(path);
        if (!fi.exists() || !fi.isFile()) {
            qWarning() << "Preview sound file does not exist:" << path;
            return;
        }

        previewPlayer->stop();
        previewPlayer->setSource(QUrl::fromLocalFile(fi.absoluteFilePath()));
        previewPlayer->play();
    });
    layout->addWidget(previewBtn);

    // 清除
    auto *clearBtn = new QPushButton(QString::fromUtf8("清除"), row);
    clearBtn->setFixedSize(48, 28);
    clearBtn->setCursor(Qt::PointingHandCursor);
    clearBtn->setFocusPolicy(Qt::NoFocus);
    clearBtn->setStyleSheet(btnStyle);
    connect(clearBtn, &QPushButton::clicked, this, [this, eventKey, updateFileLabel]() {
        AppConfig::instance().setSoundForEvent(eventKey, QString());
        AppConfig::instance().save();
        updateFileLabel();
    });
    layout->addWidget(clearBtn);

    return row;
}
