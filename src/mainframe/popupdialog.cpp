#include "popupdialog.h"

PopupDialog::PopupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(QSize(1,1));
    move(0,0);
}
