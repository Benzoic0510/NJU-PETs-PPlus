//
// Created by BenzoicAcid on 2026/5/15.
//

#ifndef SCHEDULEEDITPANEL_H
#define SCHEDULEEDITPANEL_H

#include "business/NLPService.h"
#include "business/ScheduleService.h"
#include "data/models/Schedule.h"

#include <QComboBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class ScheduleEditPanel : public QWidget {
    Q_OBJECT

public:
    explicit ScheduleEditPanel(ScheduleService *svc, NLPService *nlp,
                                QWidget *parent = nullptr);

    void showForNew(const QDate &date);
    void showForEdit(const Schedule &s);

signals:
    void dismissed();

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void onParse();
    void onConfirm();
    void onNlpParsed(const Schedule &s);
    void onNlpFailed(const QString &reason);
    void onNlpClarification(const QString &question);

private:
    void setupUi();
    void fillForm(const Schedule &s);
    void setStatus(const QString &msg, bool isError = false);

    ScheduleService *m_svc;
    NLPService      *m_nlp;

    QLineEdit     *m_nlpEdit    = nullptr;
    QPushButton   *m_parseBtn   = nullptr;
    QPushButton   *m_confirmBtn = nullptr;
    QLabel        *m_status     = nullptr;
    int            m_editingId      = -1;   // -1 = new, >=0 = editing
    Schedule       m_currentSchedule;       // valid only when m_editingId >= 0

    QLineEdit     *m_titleEdit = nullptr;
    QDateTimeEdit *m_startEdit = nullptr;
    QDateTimeEdit *m_endEdit   = nullptr;
    QLineEdit     *m_locEdit   = nullptr;
    QComboBox     *m_remindBox = nullptr;
};

#endif // SCHEDULEEDITPANEL_H
