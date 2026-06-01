//
// Created by Gungnir on 2026/6/1.
//

#ifndef OTHERPANEL_H
#define OTHERPANEL_H

#include "business/ScheduleService.h"

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>
#include <QWidget>

#include <functional>

class OtherPanel : public QWidget {
    Q_OBJECT

public:
    explicit OtherPanel(ScheduleService *scheduleService, QWidget *parent = nullptr);

    void prepareGroupsEnter();
    void playGroupsEnter();
    void playGroupsExit(const std::function<void()> &finished);

public slots:
    void setCategory(int index);

private:
    void setupUi();
    QWidget *makeVersionPage();
    QWidget *makeDataPage();
    QWidget *makeHelpPage();
    QWidget *makeGroup(const QString &title);
    QWidget *makeInfoRow(const QString &label, const QString &value);
    QWidget *makeLinkRow(const QString &text, const QString &url);
    QWidget *makeActionRow(const QString &label, QWidget *control);
    QPushButton *makeActionButton(const QString &text, bool primary = false);
    QVector<QWidget *> currentGroupHosts() const;
    void clearExpiredSchedules();

    ScheduleService *m_scheduleService = nullptr;
    QStackedWidget *m_stack = nullptr;
    QVector<QWidget *> m_versionGroupHosts;
    QVector<QWidget *> m_dataGroupHosts;
    QVector<QWidget *> m_helpGroupHosts;
    QLabel *m_cleanupStatus = nullptr;
    QPushButton *m_cleanupButton = nullptr;
    int m_cleanupGeneration = 0;
    bool m_cleanupConfirming = false;
};

#endif // OTHERPANEL_H
