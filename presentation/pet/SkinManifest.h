//
// Created by LemonSugar on 2026/5/25.
//

#ifndef SKINMANIFEST_H
#define SKINMANIFEST_H

#include <QHash>
#include <QString>

struct GridInfo { int rows; int cols; };

class SkinManifest {
public:
    static SkinManifest &instance();

    GridInfo gridFor(const QString &petId, const QString &state) const;
    QStringList skins() const;

private:
    SkinManifest();
    void loadSkin(const QString &petId);

    struct StateGrid { int rows; int cols; };
    // petId → (state → grid)
    QHash<QString, QHash<QString, StateGrid>> m_skins;
};

#endif // SKINMANIFEST_H
