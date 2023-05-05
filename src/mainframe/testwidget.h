#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>

class TestWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TestWidget(QWidget *parent = nullptr);

public slots:
    void onNewPoint(double x, double y);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

private:
    QVector<QPoint> points;
};

#endif // TESTWIDGET_H
