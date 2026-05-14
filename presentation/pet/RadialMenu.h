//
// Created by BenzoicAcid on 2026/5/14.
//

#ifndef RADIALMENU_H
#define RADIALMENU_H

#include <QVector>
#include <QWidget>

class RadialMenu : public QWidget {
    Q_OBJECT

public:
    struct Item {
        QString label;
    };

    explicit RadialMenu(QWidget *parent = nullptr);

    void popup(const QPoint &globalPos, const QVector<Item> &items);

signals:
    void triggered(int index);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    int     itemAt(QPoint localPos) const;
    QPointF itemCenter(int index) const;

    QVector<Item> m_items;
    int m_hovered = -1;

    static constexpr int WidgetSize = 300;
    static constexpr int Radius     = 100
    ;   // 中心到 item 圆心距离
    static constexpr int ItemR      = 26;   // item 圆半径
};

#endif // RADIALMENU_H
