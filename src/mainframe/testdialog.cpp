#include "testdialog.h"
#include "ui_testdialog.h"
#include "apmanager.h"

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);
}

TestDialog::~TestDialog()
{
    delete ui;
}

void TestDialog::on_pushButton_clicked()
{
    QList<QString> macs;
    //macs.push_back("CC1BE0E1CED0");
    macs.push_back("CC1BE0E0E850");
    Socket socket;
    socket.client = nullptr;
    APManager::getInstance()->onAddAp(socket,macs);
}


void TestDialog::on_pushButton_2_clicked()
{
//    APManager::getInstance()->
}

void TestDialog::on_pushButton_3_clicked()
{
    APManager::getInstance()->startScan();
}

void TestDialog::on_pushButton_4_clicked()
{
}

void TestDialog::on_pushButton_5_clicked()
{
    APManager::getInstance()->stopScan();
}

void TestDialog::on_pushButton_6_clicked()
{

}

void TestDialog::on_pushButton_7_clicked()
{

}

void TestDialog::on_pushButton_8_clicked()
{

}

void TestDialog::on_pushButton_9_clicked()
{

}
