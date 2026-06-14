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
        QString icon;   // Text symbol drawn above the label.
    };

    explicit RadialMenu(QWidget *parent = nullptr);

    void popup(const QPoint &globalPos, const QVector<Item> &items);

signals:
    void triggered(int index);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    int     itemAt(QPoint localPos) const;
    void    drawSector(QPainter &p, int index) const;
    double  sectorStartAngle(int index) const;  // 返回弧度

    QVector<Item> m_items;
    int m_hovered = -1;

    static constexpr int    WidgetSize = 320;
    static constexpr double InnerR     = 68.0;
    static constexpr double OuterR     = 126.0;
    static constexpr double GapDeg     = 0.0;   // 扇形按钮之间不留角度间隙
};

#endif // RADIALMENU_H
