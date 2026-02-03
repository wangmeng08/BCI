#include "choosetype.h"
#include "ui_choosetype.h"

ChooseType::ChooseType(int &selectType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseType)
{
    ui->setupUi(this);
    connect(ui->btnPen, &QPushButton::clicked, this, &ChooseType::OnClickHIFU);
    connect(ui->btnLIFUPA, &QPushButton::clicked, this, &ChooseType::OnClickLIFU);
    connect(ui->btnLIFUC, &QPushButton::clicked, this, &ChooseType::OnClickLIFUC);
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

void ChooseType::OnClickLIFUC()
{
    *m_SelectType = 3;
    reject();
}
