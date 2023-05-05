#ifndef TIPDIALOG_H
#define TIPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>

class  CustomStyledMenu : public QDialog
{
    Q_OBJECT

public:
    explicit CustomStyledMenu(QWidget *parent = nullptr);
    ~CustomStyledMenu();
    bool eventFilter(QObject *watched, QEvent *event) override;

    void addMenuItem(QWidget* item);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVBoxLayout* m_layout;

};

#endif // TIPDIALOG_H
