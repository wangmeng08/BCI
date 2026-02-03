#include "basedialog.h"
#include <QPainter>

BaseDialog::BaseDialog(QWidget *parent)
    : QDialog{parent}
{
    setFocusPolicy(Qt::StrongFocus);
}

void BaseDialog::keyPressEvent(QKeyEvent *event)
{
    return;
}

void BaseDialog::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(0, 0, 0, 204));
}
