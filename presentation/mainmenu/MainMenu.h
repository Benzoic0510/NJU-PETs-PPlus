//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef MAINMENU_H
#define MAINMENU_H

#include "business/ScheduleService.h"

#include <QButtonGroup>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

class MainMenu : public QWidget {
    Q_OBJECT

public:
    explicit MainMenu(ScheduleService *svc, QWidget *parent = nullptr);

public slots:
    void refreshSchedules();

signals:
    void petSelected(const QString &petId);

private:
    void     setupUi(ScheduleService *svc);
    QPushButton *makeNavBtn(const QString &text);
    QWidget     *makePlaceholder(const QString &text);

    QStackedWidget *m_stack  = nullptr;
    QButtonGroup   *m_navGrp = nullptr;
};

#endif // MAINMENU_H
