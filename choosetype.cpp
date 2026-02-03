#include "choosetype.h"
#include "ui_choosetype.h"

ChooseType::ChooseType(int &selectType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseType)
{
    ui->setupUi(this);
    connect(ui->btnPen, &QPushButton::clicked, this, &ChooseType::OnClickHIFU);
    connect(ui->btnLIFU128, &QPushButton::clicked, this, &ChooseType::OnClickLIFU);
    connect(ui->btnLIFU4, &QPushButton::clicked, this, &ChooseType::OnClickLIFU4);
    m_SelectType = &selectType;
}

ChooseType::~ChooseType()
{
    delete ui;
}

void ChooseType::OnClickHIFU()
{
    *m_SelectType = 1;
    accept();
}

void ChooseType::OnClickLIFU()
{
    *m_SelectType = 2;
    reject();
}

void ChooseType::OnClickLIFU4()
{
    *m_SelectType = 3;
    reject();
}
