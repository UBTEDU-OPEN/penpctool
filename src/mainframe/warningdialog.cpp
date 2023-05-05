#include "warningdialog.h"
#include "ui_warningdialog.h"

WarningDialog::WarningDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WarningDialog)
{
    ui->setupUi(this);
}

WarningDialog::~WarningDialog()
{
    delete ui;
}

void WarningDialog::on_pushButton_clicked()
{
    emit clickResult(true);
    close();
}

void WarningDialog::on_pushButton_2_clicked()
{
    emit clickResult(false);
    close();
}
