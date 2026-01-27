#include "choosetype.h"
#include "ui_choosetype.h"

ChooseType::ChooseType(int &selectType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseType)
{
    ui->setupUi(this);
    connect(ui->btnHIFU, &QPushButton::clicked, this, &ChooseType::OnClickHIFU);
    connect(ui->btnLIFU, &QPushButton::clicked, this, &ChooseType::OnClickLIFU);
    m_SelectType = &selectType;
}

ChooseType::~ChooseType()
{
    delete ui;
}

void ChooseType::OnClickHIFU()
{
    *m_SelectType = 0;
    accept();
}

void ChooseType::OnClickLIFU()
{
    *m_SelectType = 1;
    reject();
}
