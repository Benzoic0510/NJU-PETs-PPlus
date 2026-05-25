//
// Created by LemonSugar on 2026/5/25.
//

#ifndef SKINMANIFEST_H
#define SKINMANIFEST_H

#include <QHash>
#include <QString>

struct GridInfo { int rows; int cols; };

struct SleepSegments {
    int fallingStart = 0,  fallingEnd = 0;
    int loopStart    = 0,  loopEnd    = 0;
    int wakeStart    = 0,  wakeEnd    = 0;
    bool valid = false;
};

class SkinManifest {
public:
    static SkinManifest &instance();

    GridInfo      gridFor(const QString &petId, const QString &state) const;
    SleepSegments sleepSegmentsFor(const QString &petId) const;
    QStringList   skins() const;

private:
    SkinManifest();
    void loadSkin(const QString &petId);

    struct StateGrid { int rows; int cols; };
    QHash<QString, QHash<QString, StateGrid>> m_grids;
    QHash<QString, SleepSegments>             m_sleepSegments;
};

#endif // SKINMANIFEST_H
