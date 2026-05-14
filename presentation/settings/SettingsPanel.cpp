//
// Created by BenzoicAcid on 2026/5/15.
//

#include "presentation/settings/SettingsPanel.h"
#include "data/AppConfig.h"
#include "presentation/common/Theme.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

SettingsPanel::SettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void SettingsPanel::setupUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(24);

    auto *title = new QLabel("设置");
    title->setStyleSheet(
        "font-size: 18px; font-weight: 600;"
        "color: " + QString(Theme::TextPrimary) + ";");
    root->addWidget(title);

    auto *form = new QFormLayout;
    form->setSpacing(16);
    form->setContentsMargins(0, 0, 0, 0);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // ── API Key ──────────────────────────────────────────────
    const bool envSet = !qgetenv("DASHSCOPE_API_KEY").isEmpty();

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
    toggleBtn->setCheckable(true);
    toggleBtn->setStyleSheet(
        "QPushButton { border: 1px solid " + QString(Theme::Border) + ";"
        "  border-radius: 6px; font-size: 12px; background: none;"
        "  color: " + Theme::TextSecondary + "; }"
        "QPushButton:hover { background: " + Theme::BgSecondary + "; }");
    connect(toggleBtn, &QPushButton::toggled, this, [this, toggleBtn](bool on) {
        m_apiKeyEdit->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
        toggleBtn->setText(on ? "隐藏" : "显示");
    });

    keyRow->addWidget(m_apiKeyEdit, 1);
    keyRow->addWidget(toggleBtn);
    keyVl->addLayout(keyRow);

    if (envSet) {
        auto *hint = new QLabel("已检测到环境变量 DASHSCOPE_API_KEY，运行时优先使用，此处不可编辑。");
        hint->setStyleSheet("font-size: 11px; color: " + QString(Theme::TextTertiary) + ";");
        hint->setWordWrap(true);
        keyVl->addWidget(hint);
    }
    form->addRow("API_Key", keyWidget);

    // ── 音量 ─────────────────────────────────────────────────
    auto *volWidget = new QWidget;
    auto *volRow = new QHBoxLayout(volWidget);
    volRow->setContentsMargins(0, 0, 0, 0);
    volRow->setSpacing(8);

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(AppConfig::instance().volume());

    m_volumeLabel = new QLabel(QString::number(AppConfig::instance().volume()));
    m_volumeLabel->setFixedWidth(28);
    m_volumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_volumeLabel->setStyleSheet("font-size: 12px; color: " + QString(Theme::TextSecondary) + ";");

    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int v) {
        m_volumeLabel->setText(QString::number(v));
    });

    volRow->addWidget(m_volumeSlider, 1);
    volRow->addWidget(m_volumeLabel);
    form->addRow("提醒音量", volWidget);

    // ── 提醒开关 ──────────────────────────────────────────────
    m_reminderCheck = new QCheckBox("启用日程提醒");
    m_reminderCheck->setChecked(AppConfig::instance().reminderEnabled());
    m_reminderCheck->setStyleSheet("font-size: 13px; color: " + QString(Theme::TextSecondary) + ";");
    form->addRow("日程提醒", m_reminderCheck);

    root->addLayout(form);

    // ── 保存按钮 ──────────────────────────────────────────────
    m_saveBtn = new QPushButton("保存设置");
    m_saveBtn->setFixedWidth(100);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(
        "QPushButton { background: " + QString(Theme::Primary) + "; color: white;"
        "  border-radius: 8px; font-size: 13px; padding: 7px 0; border: none; }"
        "QPushButton:hover { background: " + Theme::PrimaryDark + "; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsPanel::onSave);
    root->addWidget(m_saveBtn, 0, Qt::AlignLeft);

    root->addStretch();
}

void SettingsPanel::onSave() {
    AppConfig &cfg = AppConfig::instance();
    if (qgetenv("DASHSCOPE_API_KEY").isEmpty())
        cfg.setApiKey(m_apiKeyEdit->text().trimmed());
    cfg.setVolume(m_volumeSlider->value());
    cfg.setReminderEnabled(m_reminderCheck->isChecked());
    cfg.save();

    m_saveBtn->setText("保存成功");
    m_saveBtn->setEnabled(false);
    QTimer::singleShot(2000, this, [this]() {
        m_saveBtn->setText("保存设置");
        m_saveBtn->setEnabled(true);
    });
}
