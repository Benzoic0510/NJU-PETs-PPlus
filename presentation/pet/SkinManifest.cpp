//
// Created by LemonSugar on 2026/5/25.
//

#include "presentation/pet/SkinManifest.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

SkinManifest &SkinManifest::instance() {
    static SkinManifest inst;
    return inst;
}

SkinManifest::SkinManifest() {
    loadSkin("Muelsyse");
}

void SkinManifest::loadSkin(const QString &petId) {
    const QString path = ":/sprites/" + petId + "/pet.json";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;

    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    const QJsonObject states = root["states"].toObject();

    QHash<QString, StateGrid> map;
    for (auto it = states.begin(); it != states.end(); ++it) {
        const QJsonObject g = it.value().toObject();
        map[it.key()] = {g["rows"].toInt(1), g["cols"].toInt(1)};
    }
    m_skins[petId] = map;
}

GridInfo SkinManifest::gridFor(const QString &petId, const QString &state) const {
    const auto itPet = m_skins.constFind(petId);
    if (itPet != m_skins.constEnd()) {
        const auto itSt = itPet->constFind(state);
        if (itSt != itPet->constEnd())
            return {itSt->rows, itSt->cols};
    }
    return {1, 0};
}

QStringList SkinManifest::skins() const {
    return m_skins.keys();
}
