//
// Created by BenzoicAcid on 2026/5/15.
//

#include "presentation/selector/PetSelector.h"
#include "data/AppConfig.h"
#include "presentation/common/Theme.h"

#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

const QVector<PetSelector::PetDef> PetSelector::PETS = {
    {"Muelsyse", "Muelsyse", ":/sprites/Muelsyse_idle.png"},
};

PetSelector::PetSelector(QWidget *parent)
    : QWidget(parent)
    , m_currentId(AppConfig::instance().petId())
{
    setupUi();
}

void PetSelector::setupUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 28);
    root->setSpacing(20);

    auto *title = new QLabel("选择宠物形象");
    title->setStyleSheet(
        "font-size: 18px; font-weight: 600;"
        "color: " + QString(Theme::TextPrimary) + ";");
    root->addWidget(title);

    auto *sub = new QLabel("选择后立即生效，宠物形象会随之更换。");
    sub->setStyleSheet("font-size: 12px; color: " + QString(Theme::TextTertiary) + ";");
    root->addWidget(sub);

    auto *grid = new QWidget;
    auto *gl = new QGridLayout(grid);
    gl->setSpacing(16);
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    for (int i = 0; i < PETS.size(); ++i) {
        const PetDef &pet = PETS[i];

        auto *card = new QWidget;
        card->setFixedSize(160, 220);
        m_cards[pet.id] = card;

        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 16, 12, 12);
        cl->setSpacing(8);
        cl->setAlignment(Qt::AlignCenter);

        // 精灵图预览（取 idle 第一帧）
        auto *preview = new QLabel;
        preview->setFixedSize(100, 100);
        preview->setAlignment(Qt::AlignCenter);
        QPixmap sheet(pet.spritePath);
        if (!sheet.isNull()) {
            preview->setPixmap(sheet.copy(0, 0, 128, 128).scaled(
                100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            preview->setStyleSheet(
                "background: " + QString(Theme::PrimaryBg) + ";"
                "border-radius: 50px; font-size: 44px;");
            preview->setText("🐋");
        }
        cl->addWidget(preview, 0, Qt::AlignCenter);

        // 宠物名称
        auto *nameLbl = new QLabel(pet.name);
        nameLbl->setAlignment(Qt::AlignCenter);
        nameLbl->setStyleSheet(
            "font-size: 13px; font-weight: 500;"
            "color: " + QString(Theme::TextPrimary) + ";");
        cl->addWidget(nameLbl);

        // 选择按钮
        auto *btn = new QPushButton;
        btn->setFixedHeight(28);
        m_selectBtns[pet.id] = btn;

        const QString petId = pet.id;
        connect(btn, &QPushButton::clicked, this, [this, petId]() {
            AppConfig::instance().setPetId(petId);
            AppConfig::instance().save();
            markSelected(petId);
            emit petSelected(petId);
        });
        cl->addWidget(btn);

        gl->addWidget(card, i / 4, i % 4);
    }

    root->addWidget(grid);
    root->addStretch();

    markSelected(m_currentId);
}

void PetSelector::markSelected(const QString &petId) {
    m_currentId = petId;

    for (auto it = m_cards.begin(); it != m_cards.end(); ++it) {
        const bool sel = (it.key() == petId);
        it.value()->setStyleSheet(
            "background: " + QString(Theme::BgPrimary) + "; border-radius: 12px;"
            "border: 2px solid " + QString(sel ? Theme::Primary : Theme::Border) + ";");
    }

    for (auto it = m_selectBtns.begin(); it != m_selectBtns.end(); ++it) {
        const bool sel = (it.key() == petId);
        auto *btn = it.value();
        if (sel) {
            btn->setText("✓ 已选择");
            btn->setStyleSheet(
                "QPushButton { background: " + QString(Theme::Primary) + "; color: " + Theme::BgPrimary + ";"
                "  border: none; border-radius: 6px; font-size: 12px; }");
            btn->setEnabled(false);
        } else {
            btn->setText("选择");
            btn->setStyleSheet(
                "QPushButton { background: none; color: " + QString(Theme::Primary) + ";"
                "  border: 1px solid " + Theme::Primary + ";"
                "  border-radius: 6px; font-size: 12px; }"
                "QPushButton:hover { background: " + Theme::PrimaryBg + "; }");
            btn->setEnabled(true);
        }
    }
}
