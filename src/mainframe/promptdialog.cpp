#include "promptdialog.h"
#include "ui_promptdialog.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QStyle>

#include "projectconst.h"


PromptDialog::PromptDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PromptDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
//    setAttribute(Qt::WA_DeleteOnClose);

    setModal(true);
    setFixedSize(parent->size());
    ui->centralWidget->setFixedSize(500,230);

    closeBtn = new QPushButton(this);
    closeBtn->setFixedSize(40,40);
    closeBtn->setIcon(QIcon(":/res/close_pop.png"));
    closeBtn->setIconSize(QSize(40,40));
    closeBtn->setFlat(true);   

    connect(closeBtn,&QPushButton::clicked,this,&PromptDialog::onClose);
    connect(ui->cancel,&QPushButton::clicked,this,&PromptDialog::onCancel);
    connect(ui->ok,&QPushButton::clicked,this,&PromptDialog::onOk);
}

PromptDialog::~PromptDialog()
{
    delete ui;
}

void PromptDialog::setTitle(const QString &title)
{
    ui->title->setText(title);
}

void PromptDialog::setMsg(const QString &msg)
{
    ui->msg->setText(msg);
}

void PromptDialog::setCancelText(const QString &text)
{
    ui->cancel->setText(text);
}

void PromptDialog::setOkText(const QString &text)
{
    ui->ok->setText(text);
}

void PromptDialog::hideOkBtn()
{
    ui->ok->setEnabled(false);
    ui->ok->setVisible(false);
}

void PromptDialog::hideCancelBtn()
{
    ui->cancel->setEnabled(false);
    ui->cancel->setVisible(false);
}

void PromptDialog::onClose()
{
    emit cancelOrClose();
}

void PromptDialog::onCancel()
{
    emit cancelOrClose();
}

void PromptDialog::onOk()
{
    emit okClicked();
}

void PromptDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(0,0,0, 60)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
    auto pos = ui->centralWidget->geometry().topRight();
    closeBtn->move(pos.x()-23,pos.y()-13);
}
