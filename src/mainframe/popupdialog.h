#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <QDialog>

class PopupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PopupDialog(QWidget *parent = nullptr);
};

#endif // POPUPDIALOG_H
