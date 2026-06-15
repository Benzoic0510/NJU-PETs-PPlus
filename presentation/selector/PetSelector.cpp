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
    {"Muelsyse", "Muelsyse", ":/sprites/Muelsyse/Muelsyse.png"},
    {"Pramanix", "Pramanix", ":/sprites/Pramanix/Pramanix.png"},
};

PetSelector::PetSelector(QWidget *parent)
    : QWidget(parent)
    , m_currentId(AppConfig::instance().petId())
{
    setupUi();
}

void PetSelector::setupUi() {
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *grid = new QWidget;
    auto *gl = new QGridLayout(grid);
    gl->setHorizontalSpacing(16);
    gl->setVerticalSpacing(18);
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    for (int i = 0; i < PETS.size(); ++i) {
        const PetDef &pet = PETS[i];

        auto *card = new QWidget;
        card->setObjectName("petCard");
        card->setFixedSize(148, 198);
        m_cards[pet.id] = card;

        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 14, 12, 12);
        cl->setSpacing(8);
        cl->setAlignment(Qt::AlignCenter);

        auto *preview = new QLabel;
        preview->setObjectName("petPreview");
        preview->setFixedSize(112, 112);
        preview->setAlignment(Qt::AlignCenter);
        QPixmap image(pet.spritePath);
        if (!image.isNull()) {
            preview->setPixmap(image.scaled(
                112, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            preview->setStyleSheet("font-size: 40px; color: " + QString(Theme::TextTertiary) + ";");
            preview->setText("+");
        }
        cl->addWidget(preview, 0, Qt::AlignCenter);

        auto *nameLbl = new QLabel(pet.name);
        nameLbl->setObjectName("petName");
        nameLbl->setAlignment(Qt::AlignCenter);
        nameLbl->setStyleSheet(
            "font-size: 14px; font-weight: 700;"
            "color: " + QString(Theme::TextPrimary) + ";");
        cl->addWidget(nameLbl);

        auto *btn = new QPushButton;
        btn->setObjectName("petSelectButton");
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

    const int extraIndex = PETS.size();
    auto *pendingCard = new QWidget;
    pendingCard->setObjectName("pendingPetCard");
    pendingCard->setFixedSize(148, 198);
    pendingCard->setStyleSheet(
        "#pendingPetCard {"
        "  background: " + QString(Theme::BgPrimary) + ";"
        "  border: 1px dashed " + Theme::BorderSecondary + ";"
        "  border-radius: 10px;"
        "}"
        "#pendingPetCard QLabel {"
        "  border: none;"
        "  background: transparent;"
        "}");
    auto *pendingLayout = new QVBoxLayout(pendingCard);
    pendingLayout->setContentsMargins(12, 14, 12, 12);
    pendingLayout->setSpacing(10);
    pendingLayout->setAlignment(Qt::AlignCenter);

    auto *plus = new QLabel("+");
    plus->setObjectName("pendingPlus");
    plus->setFixedSize(112, 112);
    plus->setAlignment(Qt::AlignCenter);
    plus->setStyleSheet(
        "font-size: 44px; font-weight: 300;"
        "color: " + QString(Theme::TextTertiary) + ";");
    pendingLayout->addWidget(plus, 0, Qt::AlignCenter);

    auto *pendingText = new QLabel("未完待续");
    pendingText->setObjectName("pendingText");
    pendingText->setAlignment(Qt::AlignCenter);
    pendingText->setStyleSheet(
        "font-size: 14px; font-weight: 700;"
        "color: " + QString(Theme::TextSecondary) + ";");
    pendingLayout->addWidget(pendingText);
    pendingLayout->addStretch();
    gl->addWidget(pendingCard, extraIndex / 4, extraIndex % 4);

    root->addWidget(grid);
    root->addStretch();

    markSelected(m_currentId);
}

void PetSelector::markSelected(const QString &petId) {
    m_currentId = petId;

    for (auto it = m_cards.begin(); it != m_cards.end(); ++it) {
        const bool sel = (it.key() == petId);
        it.value()->setStyleSheet(
            "#petCard {"
            "  background: " + QString(Theme::BgPrimary) + ";"
            "  border-radius: 10px;"
            "  border: 1px solid " + QString(sel ? Theme::Primary : Theme::BorderSecondary) + ";"
            "}"
            "#petCard QLabel {"
            "  border: none;"
            "  background: transparent;"
            "}"
            "#petCard QPushButton {"
            "  border: none;"
            "}");
    }

    for (auto it = m_selectBtns.begin(); it != m_selectBtns.end(); ++it) {
        const bool sel = (it.key() == petId);
        auto *btn = it.value();
        if (sel) {
            btn->setText("✓ 已选择");
            btn->setStyleSheet(
                "#petSelectButton { background: " + QString(Theme::Primary) + "; color: " + Theme::BgPrimary + ";"
                "  border: none; border-radius: 6px; font-size: 13px; }");
            btn->setEnabled(false);
        } else {
            btn->setText("选择");
            btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet(
                "#petSelectButton { background: " + QString(Theme::PrimaryBg) + "; color: " + Theme::PrimaryDark + ";"
                "  border: none; border-radius: 6px; font-size: 13px; }"
                "#petSelectButton:hover { background: " + QString(Theme::PrimaryMid) + "; color: " + Theme::BgPrimary + "; }");
            btn->setEnabled(true);
        }
    }
}
