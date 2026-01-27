#include "option.h"

#include "datamanager.h"
#include "ui_option.h"

Option::Option(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Option)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    OnCancelClick();
    connect(ui->btnAccept, &QPushButton::clicked, this, &Option::OnAcceptClick);
    connect(ui->btnCancel, &QPushButton::clicked, this, &Option::OnCancelClick);
    connect(ui->btnBack, &QPushButton::clicked, this, &Option::OnCloseClick);

    connect(ui->cmbLimit, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPowerLimitChange(int)));
}

Option::~Option()
{
    delete ui;
}

void Option::OnAcceptClick()
{
    bool isUsePowerLimit = (ui->cmbLimit->currentIndex() == 0);
    bool isAdvanceRFMode = (ui->cmbAdvance->currentIndex() == 0);
    TriggerMode mode = static_cast<TriggerMode>(ui->cmbMode->currentIndex());
    DataManager::GetInstance()->SetOptionInfo(isUsePowerLimit, isAdvanceRFMode, mode);
}

void Option::OnCancelClick()
{
    int index = DataManager::GetInstance()->m_IsUsePowerLimit ? 0 : 1;
    ui->cmbLimit->setCurrentIndex(index);
    index = DataManager::GetInstance()->m_IsAdvanceRFMode ? 0 : 1;
    ui->cmbAdvance->setCurrentIndex(index);
    index = static_cast<int>(DataManager::GetInstance()->m_TriggerMode);
    ui->cmbMode->setCurrentIndex(index);
    ui->cmbAdvance->setEnabled(!DataManager::GetInstance()->m_IsUsePowerLimit);
}

void Option::OnCloseClick()
{
    close();
}

void Option::OnPowerLimitChange(int index)
{
    if(index == 0){
        ui->cmbAdvance->setCurrentIndex(1);
        ui->cmbAdvance->setEnabled(false);
    }
    else{
        ui->cmbAdvance->setEnabled(true);
    }
}
