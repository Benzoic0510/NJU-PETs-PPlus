//
// Created by BenzoicAcid on 2026/5/15.
//

#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QWidget>

class SettingsPanel : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);

public slots:
    void setCategory(int index);

signals:
    void petScaleChanged(int scale);

private:
    void setupUi();
    QWidget *makePetPage();
    QWidget *makeSchedulePage();
    void onSave();

    QStackedWidget *m_stack        = nullptr;
    QSlider    *m_petScaleSlider   = nullptr;
    QLabel     *m_petScaleLabel    = nullptr;
    QLineEdit  *m_apiKeyEdit    = nullptr;
    QSlider    *m_volumeSlider  = nullptr;
    QLabel     *m_volumeLabel   = nullptr;
    QCheckBox  *m_reminderCheck = nullptr;
    QPushButton *m_saveBtn      = nullptr;
};

#endif // SETTINGSPANEL_H
