#include "customstyledmenu.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QPainterPath>

CustomStyledMenu::CustomStyledMenu(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(false);
    qApp->installEventFilter(this);
    setAttribute(Qt::WA_DeleteOnClose);
    m_layout = new QVBoxLayout(this);
    setFixedWidth(100);
}

CustomStyledMenu::~CustomStyledMenu()
{
}

void CustomStyledMenu::addMenuItem(QWidget *item)
{
    m_layout->addWidget(item);
}

void CustomStyledMenu::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    QPainterPath drawPath;

    drawPath.addRoundedRect(0,0,width(),height(),7,7);
//    QPolygon trianglePolygon;
//    trianglePolygon << QPoint(170, 0);
//    trianglePolygon << QPoint(164, 6);
//    trianglePolygon << QPoint(176, 6);
//    drawPath.addPolygon(trianglePolygon);
    painter.drawPath(drawPath);
}

bool CustomStyledMenu::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
//    if(watched != this) {
        switch (event->type()) {
//        case QEvent::Leave:
//        case QEvent::WindowActivate:
//        case QEvent::WindowDeactivate:
//        case QEvent::FocusIn:
//        case QEvent::FocusOut:
        case QEvent::Close:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
//        case QEvent::Wheel:
            hide();
            break;
        default:
            break;
        }
//    }
    return false;
}
