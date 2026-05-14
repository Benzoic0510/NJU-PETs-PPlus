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
#include <QWidget>

class SettingsPanel : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);

private:
    void setupUi();
    void onSave();

    QLineEdit  *m_apiKeyEdit    = nullptr;
    QSlider    *m_volumeSlider  = nullptr;
    QLabel     *m_volumeLabel   = nullptr;
    QCheckBox  *m_reminderCheck = nullptr;
    QPushButton *m_saveBtn      = nullptr;
};

#endif // SETTINGSPANEL_H
