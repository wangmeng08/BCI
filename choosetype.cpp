#include "choosetype.h"
#include "datamanager.h"
#include "eventmanager.h"
#include "ui_choosetype.h"

ChooseType::ChooseType(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChooseType)
{
    ui->setupUi(this);
    connect(ui->btnPen, &QPushButton::clicked, this, &ChooseType::OnClickHIFU);
    connect(ui->btnLIFU128, &QPushButton::clicked, this, &ChooseType::OnClickLIFU);
    connect(ui->btnLIFU4, &QPushButton::clicked, this, &ChooseType::OnClickLIFU4);
}

ChooseType::~ChooseType()
{
    delete ui;
}

void ChooseType::OnClickHIFU()
{
    SetPage(Page::HIFU);
}

void ChooseType::OnClickLIFU()
{
    SetPage(Page::LIFU);
}

void ChooseType::OnClickLIFU4()
{
    SetPage(Page::LIFU4);
}

void ChooseType::SetPage(Page page)
{
    ClinicalMode mode = (ClinicalMode)((int)page + (int)ClinicalMode::HIFU - (int)Page::HIFU);
    DataManager::GetInstance()->SetClinicalMode(mode);
    emit EventManager::GetInstance()->turnToPage(page);
}
