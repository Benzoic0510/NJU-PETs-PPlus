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

    QHash<QString, StateGrid> grids;
    for (auto it = states.begin(); it != states.end(); ++it) {
        const QJsonObject obj = it.value().toObject();
        grids[it.key()] = {obj["rows"].toInt(1), obj["cols"].toInt(1)};
    }
    m_grids[petId] = grids;

    // 睡眠分段（可选）
    const QJsonObject sleepObj = states["sleep"].toObject();
    const QJsonObject seg      = sleepObj["segments"].toObject();
    if (!seg.isEmpty()) {
        SleepSegments ss;
        const QJsonObject falling = seg["falling"].toObject();
        const QJsonObject looping = seg["looping"].toObject();
        const QJsonObject waking  = seg["waking"].toObject();
        ss.fallingStart = falling["start"].toInt(0);
        ss.fallingEnd   = falling["end"].toInt(0);
        ss.loopStart    = looping["start"].toInt(0);
        ss.loopEnd      = looping["end"].toInt(0);
        ss.wakeStart    = waking["start"].toInt(0);
        ss.wakeEnd      = waking["end"].toInt(0);
        ss.valid        = true;
        m_sleepSegments[petId] = ss;
    }
}

GridInfo SkinManifest::gridFor(const QString &petId, const QString &state) const {
    const auto itPet = m_grids.constFind(petId);
    if (itPet != m_grids.constEnd()) {
        const auto itSt = itPet->constFind(state);
        if (itSt != itPet->constEnd())
            return {itSt->rows, itSt->cols};
    }
    return {1, 0};
}

SleepSegments SkinManifest::sleepSegmentsFor(const QString &petId) const {
    return m_sleepSegments.value(petId);
}

QStringList SkinManifest::skins() const {
    return m_grids.keys();
}
