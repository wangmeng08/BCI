#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamanager.h"
#include "logmanager.h"
#include "messageinfo.h"
#include "option.h"
#include "profileload.h"
#include "savedialog.h"

#include <qtabbar.h>

MainWindow::MainWindow(QWidget *parent)
    : BaseWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    emit writeLog(LogType::INFO, "HIFU start");

    InitData();

    InitEvent();
    SetConnectState(ConnectState::DISCONNECT);
    SetEmitState(EmitState::IDLE);
    QTimer::singleShot(5000, [=](){SetConnectState(ConnectState::CONNECT);});
}

MainWindow::~MainWindow()
{
    delete ui;
}

QLabel *MainWindow::GetConnectLabel()
{
    return ui->lblState;
}

QLabel *MainWindow::GetEmitLabel()
{
    return ui->lblEmitState;
}

void MainWindow::InitData()
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
    OnClickCancel();
    OnModeAdvanceRFChange();
    OnModePowerLimitChange();
    SetEditMode(false);
}

void MainWindow::InitEvent()
{
    connect(ui->btnAdvance, &QPushButton::clicked, this, &MainWindow::OnClickAdvance);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindow::OnClickEdit);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::OnClickLoad);
    connect(ui->btnLocal, &QPushButton::clicked, this, &MainWindow::OnClickLocal);
    connect(ui->btnOff, &QPushButton::clicked, this, &MainWindow::OnClickOff);
    connect(ui->btnOn, &QPushButton::clicked, this, &MainWindow::OnClickOn);
    connect(ui->btnOption, &QPushButton::clicked, this, &MainWindow::OnClickOption);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::OnClickSave);

    connect(m_DataManager, &DataManager::advanceRFModeChange, this, &MainWindow::OnModeAdvanceRFChange);
    connect(m_DataManager, &DataManager::powerLimitModeChange, this, &MainWindow::OnModePowerLimitChange);
    connect(m_DataManager, &DataManager::triggerModeChange, this, &MainWindow::OnModeTriggerChange);
}

void MainWindow::OnClickAdvance()
{
    ui->tabWidget->setCurrentIndex(1);
    m_IsInAdvance = true;
    SetAdvanceBtnState();
}

void MainWindow::OnClickCancel()
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
    m_IsInEdit = false;
    SetEditMode(false);
}

void MainWindow::OnClickEdit()
{
    m_IsInEdit = true;
    SetEditMode(m_IsInEdit);
}

void MainWindow::OnClickLoad()
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
    m_DataManager->m_CurrentProfile = m_DataManager->m_ProfileList[loadIndex];
    OnClickCancel();
}

void MainWindow::OnClickLocal()
{
    ui->tabWidget->setCurrentIndex(0);
    m_IsInAdvance = false;
    SetAdvanceBtnState();
}

void MainWindow::OnClickOff()
{
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
}

void MainWindow::OnClickOn()
{
    SetEmitState(EmitState::ON);
    UpdateBtnState();
}

void MainWindow::OnClickOption()
{
    Option *dialog = new Option(this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
}

void MainWindow::OnClickSave()
{
    int saveType = 0;
    SaveDialog *dialog = new SaveDialog(saveType, this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
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
        break;
    default:
        break;
    }
    if(res)
    {
        SetEditMode(false);
        OnClickCancel();
    }
}

void MainWindow::OnModeAdvanceRFChange()
{
}

void MainWindow::OnModePowerLimitChange()
{
    ui->lblWarning->setVisible(!m_DataManager->m_IsUsePowerLimit);
    QString qss = "color:red";
    if(m_DataManager->m_IsUsePowerLimit)
        qss = "";
    ui->lblPower->setStyleSheet(qss);
    ui->lblIsppa->setStyleSheet(qss);
    ui->lblIspta->setStyleSheet(qss);
}

void MainWindow::OnModeTriggerChange()
{

}

void MainWindow::SetAdvanceBtnState()
{
    ui->btnAdvance->setVisible(!m_IsInAdvance);
    ui->btnLocal->setVisible(m_IsInAdvance);
}

void MainWindow::SetEditMode(bool isEdit)
{
    m_IsInEdit = isEdit;
    ui->btnSave->setVisible(isEdit);
    ui->lblDepth->setEnabled(isEdit);
    ui->lblIsppa->setEnabled(isEdit);
    ui->lblVoltage->setEnabled(isEdit);
    ui->lblTimer->setEnabled(isEdit);
    ui->lblDC->setEnabled(isEdit);
    ui->lblPeriod->setEnabled(isEdit);
    ui->lblName->setEnabled(isEdit);
    ui->lblVoltage2->setEnabled(isEdit);
    ui->lblTimer2->setEnabled(isEdit);
    ui->lblPeriod2->setEnabled(isEdit);
    ui->lblName2->setEnabled(isEdit);
    ui->lblBurstLen->setEnabled(isEdit);
    for(int i=0; i<4; i++)
    {
        m_VectorListDelay[i]->setEnabled(isEdit);
        m_VectorListFreq[i]->setEnabled(isEdit);
        m_VectorListHCD[i]->setEnabled(isEdit);
    }
    UpdateBtnState();
}

void MainWindow::UpdateBtnState()
{
    ui->btnAdvance->setVisible(false);
    ui->btnLocal->setVisible(false);
    ui->btnEdit->setVisible(false);
    ui->btnLoad->setVisible(false);
    ui->btnOn->setVisible(false);
    ui->btnOff->setVisible(false);
    ui->btnOption->setVisible(false);
    ui->btnSave->setVisible(false);
    if(m_IsInEdit)
    {
        ui->btnSave->setVisible(true);
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

