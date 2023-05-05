#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QStyle>

#include "projectconst.h"
#include "config.h"


AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->version->setText(QString::fromLocal8Bit("关于 小优AI练字 版本%1").arg(Config::localVersion()));
    ui->version1->setText(QString::fromLocal8Bit("版本号：%1").arg(Config::localVersion()));

    setModal(true);
    setFixedSize(parent->size());
    ui->aboutWidget->setFixedSize(610,400);

    closeBtn = new QPushButton(this);
    closeBtn->setFixedSize(40,40);
    closeBtn->setIcon(QIcon(":/res/close_pop.png"));
    closeBtn->setIconSize(QSize(40,40));
    closeBtn->setFlat(true);   

    connect(closeBtn,&QPushButton::clicked,this,&AboutDialog::onClose);
    connect(ui->officalSite,&QPushButton::clicked,this,&AboutDialog::onOfficalSite);
    connect(ui->license,&QPushButton::clicked,this,&AboutDialog::onLicense);
    connect(ui->privacy,&QPushButton::clicked,this,&AboutDialog::onPrivacy);
    connect(ui->upgrade,&QPushButton::clicked,this,&AboutDialog::onUpgrade);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onNewVersion(bool isForced, QString packageMd5, QString packageUrl, QString releaseNote, qint64 packageSize)
{
    isForced_ = isForced;
    packageMd5_ = packageMd5;
    packageUrl_ = packageUrl;
    releaseNote_ = releaseNote;
    packageSize_ = packageSize;
    ui->upgrade->setEnabled(true);
    ui->upgrade->setText(QString::fromLocal8Bit("升级"));
    ui->upgrade->setStyleSheet("background-image: url(:/res/btn_enable.png);border-radius: 22px;color: white;");
    ui->upgrade->style()->unpolish(ui->upgrade);
    ui->upgrade->style()->polish(ui->upgrade);
}

void AboutDialog::onUpgrade()
{
    emit upgrade(isForced_,packageMd5_,packageUrl_,releaseNote_,packageSize_);
}

void AboutDialog::setAlgoText(QString label)
{
    ui->algoVersion->setText(label);
}

void AboutDialog::onClose()
{
    emit closeAbout();
}

void AboutDialog::onOfficalSite()
{
//    QDesktopServices::openUrl(QUrl(QString::fromStdString(OFFICIAL_URL)));
}

void AboutDialog::onLicense()
{
//    QDesktopServices::openUrl(QUrl(QString::fromStdString(LICENSE_URL)));
}

void AboutDialog::onPrivacy()
{
//    QDesktopServices::openUrl(QUrl(QString::fromStdString(PRIVACY_URL)));
}

void AboutDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(0,0,0, 60)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
    auto pos = ui->aboutWidget->geometry().topRight();
    closeBtn->move(pos.x()-23,pos.y()-13);
}
