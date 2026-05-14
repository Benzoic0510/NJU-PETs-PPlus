//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef MINICALENDAR_H
#define MINICALENDAR_H

#include <QDate>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QWidget>

class MiniCalendar : public QWidget {
    Q_OBJECT

public:
    explicit MiniCalendar(QWidget *parent = nullptr);

    QDate selectedDate() const { return m_selected; }
    void  setEventDates(const QSet<QDate> &dates);
    QSize sizeHint() const override;

public slots:
    void selectDate(const QDate &date);

signals:
    void dateSelected(const QDate &date);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void   prevMonth();
    void   nextMonth();
    void   repositionNav();
    QRectF cellRect(int col, int row) const;
    QDate  dateFromCell(int col, int row) const;

    QDate        m_month;      // 当前显示月份的第一天
    QDate        m_selected;
    QSet<QDate>  m_eventDates;

    QPushButton *m_prevBtn    = nullptr;
    QLabel      *m_titleLabel = nullptr;
    QPushButton *m_nextBtn    = nullptr;

    static constexpr int NavH  = 36;
    static constexpr int DowH  = 22;
    static constexpr int CellH = 30;
};

#endif // MINICALENDAR_H
