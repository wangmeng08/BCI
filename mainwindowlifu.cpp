#include "mainwindowlifu.h"
#include "ui_mainwindowlifu.h"
#include "constvalue.h"
#include "profileload.h"
#include "savedialog.h"
#include <QListWidgetItem>

MainWindowLIFU::MainWindowLIFU(QWidget *parent) :
    BaseWindow(parent),
    ui(new Ui::MainWindowLIFU)
{
    ui->setupUi(this);
    InitData();
    InitEvent();
    SetConnectState(ConnectState::DISCONNECT);
    SetEmitState(EmitState::IDLE);
    QTimer::singleShot(1000, [=](){SendInitCommand();});
}

MainWindowLIFU::~MainWindowLIFU()
{
    delete ui;
}

void MainWindowLIFU::InitProfileData()
{
    OnClickCancel();
}

QLabel *MainWindowLIFU::GetConnectLabel()
{
    return ui->lblState;
}

QLabel *MainWindowLIFU::GetEmitLabel()
{
    return ui->lblEmitState;
}

QLabel *MainWindowLIFU::GetStateIcon()
{
    return ui->lblStateIcon;
}

void MainWindowLIFU::SendInitCommand()
{
    SendCommandSystemModel();
    OnCurrentProfileChange(nullptr, m_DataManager->m_CurrentProfileLIFU);
}

void MainWindowLIFU::SetTimerInfo()
{
    ui->lblTimer->setText(QString("%1").arg(m_CurrentTime));
}

void MainWindowLIFU::InitData()
{

    ui->listWidget->setFlow(QListView::LeftToRight);
    ui->listWidget->setWrapping(true);
    ui->listWidget->setResizeMode(QListView::Adjust);
    for(int i=0; i<ProfileLIFU::ValueCount; i++)
    {
        TXItem *widget = new TXItem(i, this);
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        ui->listWidget->setItemWidget(item, widget);
        ui->listWidget->addItem(item);
        item->setSizeHint(QSize(208, 50));
        m_VectorItem.append(widget);
    }
    SetEditMode(false);
}

void MainWindowLIFU::InitEvent()
{
    connect(ui->btnCancel, &QPushButton::clicked, this, &MainWindowLIFU::OnClickCancel);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindowLIFU::OnClickEdit);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindowLIFU::OnClickLoad);
    connect(ui->btnOff, &QPushButton::clicked, this, &MainWindowLIFU::OnClickOff);
    connect(ui->btnOn, &QPushButton::clicked, this, &MainWindowLIFU::OnClickOn);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindowLIFU::OnClickSave);
}

void MainWindowLIFU::OnClickCancel()
{
    for(int i=0; i<ProfileLIFU::ValueCount; i++)
    {
        m_VectorItem[i]->SetInfo(m_DataManager->m_CurrentProfileLIFU->values[i]);
    }
    ui->lblDutyc->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU->dutyc));
    ui->lblPeriod->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU->period));
    ui->lblTemp->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU->temp));
    ui->lblTimer->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU->timer));
    ui->lblVoltage->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU->voltage));
    ui->lblName->setText(m_DataManager->m_CurrentProfileLIFU->profileName);

    m_IsInEdit = false;
    SetEditMode(false);
}

void MainWindowLIFU::OnClickEdit()
{
    m_IsInEdit = true;
    SetEditMode(m_IsInEdit);
}

void MainWindowLIFU::OnClickLoad()
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
    QSharedPointer<ProfileLIFU> pre = QSharedPointer<ProfileLIFU>::create();
    pre->CopyInfo(m_DataManager->m_CurrentProfileLIFU.get());
    if(loadIndex < ConstValue::GetInstance()->DeleteLimit)
    {
        m_DataManager->m_CurrentProfileLIFU = m_DataManager->m_ProfileListLIFU[loadIndex];
        OnClickCancel();
    }
    else if(loadIndex == ConstValue::GetInstance()->LoadInfo)
    {
        OnClickEdit();
        for(int i=0; i<ProfileLIFU::ValueCount; i++)
        {
            m_VectorItem[i]->SetInfo(m_DataManager->m_TempLIFUValue[i]);
        }
    }
    else
    {
        int index = loadIndex - ConstValue::GetInstance()->DeleteLimit;
        if(m_DataManager->m_CurrentProfileLIFU == m_DataManager->m_ProfileListLIFU[index])
        {
            m_DataManager->m_CurrentProfileLIFU = m_DataManager->m_ProfileListLIFU[0];
            OnClickCancel();
        }
        m_DataManager->DeleteProfileLIFU(m_DataManager->m_ProfileListLIFU[index]);
    }

    OnCurrentProfileChange(pre, m_DataManager->m_CurrentProfileLIFU);
}

void MainWindowLIFU::OnClickSave()
{
    int saveType = 0;
    QString saveName = "";
    SaveDialog *dialog = new SaveDialog(saveType, saveName, this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
    QSharedPointer<ProfileLIFU> pre = QSharedPointer<ProfileLIFU>::create();
    pre->CopyInfo(m_DataManager->m_CurrentProfileLIFU.get());
    QSharedPointer<ProfileLIFU> profile = QSharedPointer<ProfileLIFU>::create();
    profile->profileName = ui->lblName->text();
    profile->dutyc = ui->lblDutyc->text().toDouble();
    profile->period = ui->lblPeriod->text().toInt();
    profile->timer = ui->lblTimer->text().toInt();
    profile->temp = ui->lblTemp->text().toDouble();
    profile->voltage = ui->lblVoltage->text().toDouble();
    for(int i=0; i<ProfileLIFU::ValueCount; i++)
    {
        profile->values[i] = m_VectorItem[i]->GetInfo();
    }
    bool res = true;
    switch(saveType)
    {
    case(0):
        res = m_DataManager->SaveInfoToCurrentProfileLIFU(profile);
        break;
    case(1):
        profile->profileName = saveName;
        res = m_DataManager->SaveInfoToNewProfileLIFU(profile);
        break;
    case(2):
        res = m_DataManager->SaveInfoToDefaultProfileLIFU(profile);
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

    OnCurrentProfileChange(pre, m_DataManager->m_CurrentProfileLIFU);
}

void MainWindowLIFU::OnCurrentProfileChange(QSharedPointer<ProfileLIFU> prev, QSharedPointer<ProfileLIFU> curr)
{
    if(prev.isNull() || prev->voltage != curr->voltage)
    {
        SendCommandSetHvout(curr->voltage);
    }
    if(prev.isNull())
    {
        SendCommandSetChannelSwitch();
    }
    if(prev.isNull())
    {
        SendCommandSetFrequency(500);
    }
    QVector<uint32_t> result = GetChannelDelay();
    SendCommandSetChannelDelay(result);
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

void MainWindowLIFU::SetEditMode(bool isEdit)
{
    m_IsInEdit = isEdit;
    for(int i=0; i<m_VectorItem.size(); i++)
    {
        m_VectorItem[i]->SetItemEnable(isEdit);
    }
    ui->lblDutyc->setEnabled(isEdit);
    ui->lblName->setEnabled(isEdit);
    ui->lblPeriod->setEnabled(isEdit);
    ui->lblTemp->setEnabled(isEdit);
    ui->lblTimer->setEnabled(isEdit);
    ui->lblVoltage->setEnabled(isEdit);
    UpdateBtnState();
}

void MainWindowLIFU::UpdateBtnState()
{
    ui->btnEdit->setVisible(false);
    ui->btnLoad->setVisible(false);
    ui->btnOn->setVisible(false);
    ui->btnOff->setVisible(false);
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
            ui->btnEdit->setVisible(true);
            ui->btnLoad->setVisible(true);
            ui->btnOn->setVisible(true);
        }
    }
}

QVector<uint32_t> MainWindowLIFU::GetChannelDelay()
{
    QVector<uint32_t> result;
    result.fill(0, 128);
    for(int i=0; i<m_VectorItem.size(); i++)
    {
        uint32_t delay = m_VectorItem[i]->GetInfo();
        result[i] = delay;
    }
    return result;
}
