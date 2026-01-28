#include "txitem.h"
#include "ui_txitem.h"

TXItem::TXItem(int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TXItem)
{
    ui->setupUi(this);
    ui->lblTitle->setText(QString("ch%1:").arg(index));
    SetItemEnable(false);
}

TXItem::~TXItem()
{
    delete ui;
}

void TXItem::SetInfo(int num)
{
    ui->lblInfo->setText(QString("%1").arg(num));
}

void TXItem::SetItemEnable(bool state)
{
    ui->lblInfo->setEnabled(state);
}

int TXItem::GetInfo()
{
    int num = ui->lblInfo->text().toInt();
    return num;
}
