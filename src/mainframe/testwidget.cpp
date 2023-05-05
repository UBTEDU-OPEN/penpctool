#include "testwidget.h"

#include <QPainter>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>
#include <QtMath>


TestWidget::TestWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(400,400);
}

void TestWidget::onNewPoint(double x, double y)
{
    QPoint p(x,y);

    points.push_back(p);
}

void TestWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(QRect(0,0,100,100),Qt::red);
    QLine line;
    for(QPoint p : points) {
            painter.drawPoint(p);
    }


}
