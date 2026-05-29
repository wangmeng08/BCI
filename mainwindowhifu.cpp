#include "mainwindowhifu.h"
#include "ui_mainwindowhifu.h"
#include "constvalue.h"
#include "datamanager.h"
#include "eventmanager.h"
#include "logmanager.h"
#include "messageinfo.h"
#include "option.h"
#include "profileload.h"
#include "savedialog.h"

#include <qtabbar.h>

MainWindowHIFU::MainWindowHIFU(QWidget *parent)
    : BaseWindow(parent)
    , ui(new Ui::MainWindowHIFU)
{
    ui->setupUi(this);
    emit EventManager::GetInstance()->writeLog(LogType::INFO, "HIFU start");

    InitData();
    InitEvent();
    SetConnectState(ConnectState::DISCONNECT);
    SetEmitState(EmitState::IDLE);
    //QTimer::singleShot(5000, [=](){SetConnectState(ConnectState::CONNECT);});
}

MainWindowHIFU::~MainWindowHIFU()
{
    delete ui;
}

void MainWindowHIFU::InitProfileData()
{
    OnClickCancel();
}

QLabel *MainWindowHIFU::GetConnectLabel()
{
    return ui->lblState;
}

QLabel *MainWindowHIFU::GetEmitLabel()
{
    return ui->lblEmitState;
}

QLabel *MainWindowHIFU::GetStateIcon()
{
    return ui->lblStateIcon;
}

void MainWindowHIFU::SendInitCommand()
{
    SendCommandSystemModel();
    OnCurrentProfileChange(nullptr, m_DataManager->m_CurrentProfile);
}

void MainWindowHIFU::SetTimerInfo()
{
    ui->lblTimer->setText(QString("%1").arg(m_CurrentTime));
    ui->lblTimer2->setText(QString("%1").arg(m_CurrentTime));
}

void MainWindowHIFU::InitData()
{
    ui->tabWidget->tabBar()->hide();
    ui->tabWidget->setCurrentIndex(0);
    m_VectorListDelay.append(ui->lblDelay1);
    m_VectorListDelay.append(ui->lblDelay2);
    m_VectorListDelay.append(ui->lblDelay3);
    m_VectorListDelay.append(ui->lblDelay4);
    m_VectorListFreq.append(ui->lblFreq1);
    m_VectorListFreq.append(ui->lblFreq2);
    m_VectorListFreq.append(ui->lblFreq3);
    m_VectorListFreq.append(ui->lblFreq4);
    m_VectorListHCD.append(ui->lblHCD1);
    m_VectorListHCD.append(ui->lblHCD2);
    m_VectorListHCD.append(ui->lblHCD3);
    m_VectorListHCD.append(ui->lblHCD4);
    OnModeAdvanceRFChange();
    OnModePowerLimitChange();
    SetEditMode(false);
}

void MainWindowHIFU::InitEvent()
{
    connect(ui->btnAdvance, &QPushButton::clicked, this, &MainWindowHIFU::OnClickAdvance);
    connect(ui->btnCancel, &QPushButton::clicked, this, &MainWindowHIFU::OnClickCancel);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindowHIFU::OnClickEdit);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindowHIFU::OnClickLoad);
    connect(ui->btnLocal, &QPushButton::clicked, this, &MainWindowHIFU::OnClickLocal);
    connect(ui->btnOff, &QPushButton::clicked, this, &MainWindowHIFU::OnClickOff);
    connect(ui->btnOn, &QPushButton::clicked, this, &MainWindowHIFU::OnClickOn);
    connect(ui->btnOption, &QPushButton::clicked, this, &MainWindowHIFU::OnClickOption);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindowHIFU::OnClickSave);

    connect(m_DataManager, &DataManager::advanceRFModeChange, this, &MainWindowHIFU::OnModeAdvanceRFChange);
    connect(m_DataManager, &DataManager::powerLimitModeChange, this, &MainWindowHIFU::OnModePowerLimitChange);
    connect(m_DataManager, &DataManager::triggerModeChange, this, &MainWindowHIFU::OnModeTriggerChange);
}

void MainWindowHIFU::OnClickAdvance()
{
    ui->tabWidget->setCurrentIndex(1);
    m_IsInAdvance = true;
    SetAdvanceBtnState();
}

void MainWindowHIFU::OnClickCancel()
{
    ui->lblDC->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->dc));
    ui->lblDepth->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->depth));
    ui->lblFreq->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->freq));
    ui->lblIsppa->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->isppa));
    ui->lblIspta->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->GetIspta()));
    ui->lblPeriod->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->period));
    ui->lblPeriod2->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->period));
    ui->lblPower->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->GetPower()));
    ui->lblTemp->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->temp));
    ui->lblTemp2->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->temp));
    ui->lblTimer->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->timer));
    ui->lblTimer2->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->timer));
    ui->lblVoltage->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->voltage));
    ui->lblVoltage2->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->voltage));
    ui->lblName->setText(m_DataManager->m_CurrentProfile->profileName);
    ui->lblName2->setText(m_DataManager->m_CurrentProfile->profileName);
    ui->lblBurstLen->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->burstLen));
    for(int i=0; i<4; i++)
    {
        m_VectorListDelay[i]->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->infoList[i]->delay));
        m_VectorListFreq[i]->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->infoList[i]->freq));
        m_VectorListHCD[i]->setText(QString("%1").arg(m_DataManager->m_CurrentProfile->infoList[i]->hcd));
    }
    SetEditMode(false);
}

void MainWindowHIFU::OnClickEdit()
{
    m_IsInEdit = true;
    SetEditMode(m_IsInEdit);
}

void MainWindowHIFU::OnClickLoad()
{
    int loadIndex = -1;
    auto dialog = new ProfileLoad(loadIndex, this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
    if(loadIndex == -1)
        return;
    QSharedPointer<Profile> pre = QSharedPointer<Profile>::create();
    pre->CopyInfo(m_DataManager->m_CurrentProfile.get());
    if(loadIndex < ConstValue::GetInstance()->DeleteLimit)
    {
        m_DataManager->m_CurrentProfile = m_DataManager->m_ProfileList[loadIndex];
        OnClickCancel();
    }
    else
    {
        int index = loadIndex - ConstValue::GetInstance()->DeleteLimit;
        if(m_DataManager->m_CurrentProfile == m_DataManager->m_ProfileList[index])
        {
            m_DataManager->m_CurrentProfile = m_DataManager->m_ProfileList[0];
            OnClickCancel();
        }
        m_DataManager->DeleteProfile(m_DataManager->m_ProfileList[index]);
    }
    OnCurrentProfileChange(pre, m_DataManager->m_CurrentProfile);
}

void MainWindowHIFU::OnClickLocal()
{
    ui->tabWidget->setCurrentIndex(0);
    m_IsInAdvance = false;
    SetAdvanceBtnState();
}

void MainWindowHIFU::OnClickOption()
{
    Option *dialog = new Option(this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
}

void MainWindowHIFU::OnClickSave()
{
    int saveType = 0;
    QString saveName = "";
    SaveDialog *dialog = new SaveDialog(saveType, saveName, this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
    QSharedPointer<Profile> pre = QSharedPointer<Profile>::create();
    pre->CopyInfo(m_DataManager->m_CurrentProfile.get());
    QSharedPointer<Profile> profile = QSharedPointer<Profile>::create();
    profile->profileName = ui->lblName->text();
    profile->isppa = ui->lblIsppa->text().toDouble();
    profile->period = ui->lblPeriod->text().toInt();
    profile->dc = ui->lblDC->text().toInt();
    profile->timer = ui->lblTimer->text().toInt();
    profile->depth = ui->lblDepth->text().toDouble();
    profile->freq = ui->lblFreq->text().toDouble();
    profile->temp = ui->lblTemp->text().toDouble();
    profile->voltage = ui->lblVoltage->text().toDouble();
    if(m_IsInAdvance)
    {
        profile->profileName = ui->lblName2->text();
        profile->period = ui->lblPeriod2->text().toInt();
        profile->timer = ui->lblTimer2->text().toInt();
        profile->voltage = ui->lblVoltage2->text().toDouble();
    }
    else
    {
        profile->profileName = ui->lblName->text();
        profile->period = ui->lblPeriod->text().toInt();
        profile->timer = ui->lblTimer->text().toInt();
        profile->voltage = ui->lblVoltage->text().toDouble();
    }
    for(int i=0; i<4; i++)
    {
        profile->infoList[i]->delay = m_VectorListDelay[i]->text().toDouble();
        profile->infoList[i]->freq = m_VectorListFreq[i]->text().toDouble();
        profile->infoList[i]->hcd = m_VectorListHCD[i]->text().toDouble();
    }
    bool res = true;
    switch(saveType)
    {
    case(0):
        res = m_DataManager->SaveInfoToCurrentProfile(profile);
        break;
    case(1):
        profile->profileName = saveName;
        res = m_DataManager->SaveInfoToNewProfile(profile);
        break;
    case(2):
        res = m_DataManager->SaveInfoToDefaultProfile(profile);
        if(res)
        {
            ui->lblName->setText(profile->profileName);
        }
        break;
    case(3):
        res = false;
        break;
    default:
        break;
    }
    if(res)
    {
        SetEditMode(false);
        OnClickCancel();
    }
    OnCurrentProfileChange(pre, m_DataManager->m_CurrentProfile);
}

void MainWindowHIFU::OnCurrentProfileChange(QSharedPointer<Profile> prev, QSharedPointer<Profile> curr)
{
    if(prev.isNull() || prev->voltage != curr->voltage)
    {
        SendCommandSetHvout(curr->voltage);
    }
    if(prev.isNull() || prev->freq != curr->freq)
    {
        SendCommandSetFrequency(curr->freq);
    }
    if(prev.isNull() || prev->period != curr->period)
    {
        SendCommandSetPri(curr->period);
    }
    if(prev.isNull())
    {
        SendCommandSetPD(50);
        SendCommandSystemTriggerModel();
    }
    if(prev.isNull() || prev->timer != curr->timer)
    {
        SendCommandSetEmitTime(curr->timer);
    }
}

void MainWindowHIFU::OnModeAdvanceRFChange()
{
}

void MainWindowHIFU::OnModePowerLimitChange()
{
    ui->lblWarning->setVisible(!m_DataManager->m_IsUsePowerLimit);
    QString qss = "color:red";
    if(m_DataManager->m_IsUsePowerLimit)
        qss = "";
    ui->lblPower->setStyleSheet(qss);
    ui->lblIsppa->setStyleSheet(qss);
    ui->lblIspta->setStyleSheet(qss);
}

void MainWindowHIFU::OnModeTriggerChange()
{
    SendCommandSystemTriggerModel();
}

void MainWindowHIFU::SendSetAfterSave(bool isInit)
{

}

void MainWindowHIFU::SetAdvanceBtnState()
{
    ui->btnAdvance->setVisible(!m_IsInAdvance);
    ui->btnLocal->setVisible(m_IsInAdvance);
}

void MainWindowHIFU::SetEditMode(bool isEdit)
{
    m_IsInEdit = isEdit;
    ui->btnSave->setVisible(isEdit);
    SetLineEditState(ui->lblDepth,isEdit);
    SetLineEditState(ui->lblIsppa, isEdit);
    SetLineEditState(ui->lblVoltage, isEdit);
    SetLineEditState(ui->lblTimer, isEdit);
    SetLineEditState(ui->lblDC, isEdit);
    SetLineEditState(ui->lblPeriod, isEdit);
    SetLineEditState( ui->lblName, isEdit);
    SetLineEditState(ui->lblVoltage2, isEdit);
    SetLineEditState(ui->lblTimer2, isEdit);
    SetLineEditState( ui->lblPeriod2, isEdit);
    SetLineEditState(ui->lblName2, isEdit);
    SetLineEditState(ui->lblBurstLen, isEdit);
    for(int i=0; i<4; i++)
    {
        SetLineEditState(m_VectorListDelay[i], isEdit);
        SetLineEditState(m_VectorListFreq[i], isEdit);
        SetLineEditState(m_VectorListHCD[i], isEdit);
    }
    UpdateBtnState();
}

void MainWindowHIFU::UpdateBtnState()
{
    ui->btnAdvance->setVisible(false);
    ui->btnLocal->setVisible(false);
    ui->btnEdit->setVisible(false);
    ui->btnLoad->setVisible(false);
    ui->btnOn->setVisible(false);
    ui->btnOff->setVisible(false);
    ui->btnOption->setVisible(false);
    ui->btnSave->setVisible(false);
    ui->btnCancel->setVisible(false);
    if(m_IsInEdit)
    {
        ui->btnSave->setVisible(true);
        ui->btnCancel->setVisible(true);
    }
    else
    {
        if(m_State == EmitState::ON)
        {
            ui->btnOff->setVisible(true);
        }
        else
        {
            SetAdvanceBtnState();
            ui->btnEdit->setVisible(true);
            ui->btnLoad->setVisible(true);
            ui->btnOn->setVisible(true);
            ui->btnOption->setVisible(true);
        }
    }
}

