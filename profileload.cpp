#include "profileload.h"
#include "ui_profileload.h"
#include "datamanager.h"

ProfileLoad::ProfileLoad(int &loadIndex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileLoad)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    if(DataManager::GetInstance()->GetClinicalMode() == ClinicalMode::HIFU)
    {
        for(int i=0; i<DataManager::GetInstance()->m_ProfileList.size(); i++)
        {
            auto profile = DataManager::GetInstance()->m_ProfileList[i];
            ui->cmbProfile->addItem(profile->profileName);
            if(profile == DataManager::GetInstance()->m_CurrentProfile)
            {
                m_CurrentIndex = i;
            }
        }
        ui->cmbProfile->setCurrentIndex(m_CurrentIndex);
    }
    else if(DataManager::GetInstance()->GetClinicalMode() == ClinicalMode::LIFU)
    {
        for(int i=0; i<DataManager::GetInstance()->m_ProfileListLIFU.size(); i++)
        {
            auto profile = DataManager::GetInstance()->m_ProfileListLIFU[i];
            ui->cmbProfile->addItem(profile->profileName);
            if(profile == DataManager::GetInstance()->m_CurrentProfileLIFU)
            {
                m_CurrentIndex = i;
            }
        }
        ui->cmbProfile->setCurrentIndex(m_CurrentIndex);
    }
    m_LoadIndex = &loadIndex;
    connect(ui->btnCancel, &QPushButton::clicked, this, &ProfileLoad::OnClickCancle);
    connect(ui->btnLoad, &QPushButton::clicked, this, &ProfileLoad::OnClickLoad);
}

ProfileLoad::~ProfileLoad()
{
    delete ui;
}

void ProfileLoad::OnClickCancle()
{
    *m_LoadIndex = -1;
    reject();
}

void ProfileLoad::OnClickLoad()
{
    if(m_CurrentIndex == ui->cmbProfile->currentIndex())
    {
        OnClickCancle();
        return;
    }
    *m_LoadIndex = ui->cmbProfile->currentIndex();
    accept();
}
