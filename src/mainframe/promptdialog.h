#ifndef PROMPTDIALOG_H
#define PROMPTDIALOG_H

#include <QDialog>

namespace Ui {
class PromptDialog;
}

class PromptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PromptDialog(QWidget *parent = nullptr);
    ~PromptDialog();

    void setTitle(const QString& title);
    void setMsg(const QString& msg);
    void setCancelText(const QString& text);
    void setOkText(const QString& text);
    void hideOkBtn();
    void hideCancelBtn();

private slots:
    void onClose();
    void onCancel();
    void onOk();

signals:
    void okClicked();
    void cancelOrClose();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::PromptDialog *ui;
    QPushButton* closeBtn;
};

#endif // PROMPTDIALOG_H
