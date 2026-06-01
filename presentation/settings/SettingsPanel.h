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
#include <QVector>
#include <QWidget>

#include <functional>

class SettingsPanel : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);
    void prepareGroupsEnter();
    void playGroupsEnter();
    void playGroupsExit(const std::function<void()> &finished);

public slots:
    void setCategory(int index);

signals:
    void petScaleChanged(int scale);
    void petSleepThresholdChanged(int mins);
    void petInteractionDisabledChanged(bool disabled);

private:
    void setupUi();
    QWidget *makePetPage();
    QWidget *makeSchedulePage();
    QWidget *makeGroup(const QString &title);
    QWidget *makeSettingRow(const QString &label, QWidget *control);
    QVector<QWidget *> currentGroupHosts() const;
    void onSave();

    QStackedWidget *m_stack        = nullptr;
    QVector<QWidget *> m_petGroupHosts;
    QVector<QWidget *> m_scheduleGroupHosts;
    QSlider    *m_petScaleSlider   = nullptr;
    QLabel     *m_petScaleLabel    = nullptr;
    QSlider    *m_sleepThresholdSlider = nullptr;
    QLabel     *m_sleepThresholdLabel  = nullptr;
    QCheckBox  *m_interactionDisabledCheck = nullptr;
    QLineEdit  *m_apiKeyEdit    = nullptr;
    QCheckBox  *m_reminderCheck = nullptr;
    QPushButton *m_saveBtn      = nullptr;
};

#endif // SETTINGSPANEL_H
