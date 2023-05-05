#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();
    void onNewVersion(bool isForced, QString packageMd5, QString packageUrl, QString releaseNote, qint64 packageSize);
    void onUpgrade();
    void setAlgoText(QString label);

private slots:
    void onClose();
    void onOfficalSite();
    void onLicense();
    void onPrivacy();

signals:
    void upgrade(bool isForced, QString packageMd5, QString packageUrl, QString releaseNote, qint64 packageSize);
    void closeAbout();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::AboutDialog *ui;
    QPushButton* closeBtn;
    bool isForced_;
    QString packageMd5_;
    QString packageUrl_;
    QString releaseNote_;
    qint64 packageSize_;
};

#endif // ABOUTDIALOG_H
