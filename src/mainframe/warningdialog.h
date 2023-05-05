#ifndef WARNINGDIALOG_H
#define WARNINGDIALOG_H

#include <QDialog>

namespace Ui {
class WarningDialog;
}

class WarningDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WarningDialog(QWidget *parent = nullptr);
    ~WarningDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

signals:
    void clickResult(bool);

private:
    Ui::WarningDialog *ui;
};

#endif // WARNINGDIALOG_H
