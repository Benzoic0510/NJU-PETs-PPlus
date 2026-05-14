//
// Created by BenzoicAcid on 2026/5/15.
//

#ifndef PETSELECTOR_H
#define PETSELECTOR_H

#include <QMap>
#include <QPushButton>
#include <QString>
#include <QWidget>

class PetSelector : public QWidget {
    Q_OBJECT

public:
    explicit PetSelector(QWidget *parent = nullptr);

signals:
    void petSelected(const QString &petId);

private:
    struct PetDef {
        QString id;
        QString name;
        QString spritePath;
    };
    static const QVector<PetDef> PETS;

    void setupUi();
    void markSelected(const QString &petId);

    QString                      m_currentId;
    QMap<QString, QWidget *>     m_cards;
    QMap<QString, QPushButton *> m_selectBtns;
};

#endif // PETSELECTOR_H
