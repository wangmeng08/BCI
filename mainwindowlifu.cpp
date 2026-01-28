#include "mainwindowlifu.h"
#include "ui_mainwindowlifu.h"
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
}

MainWindowLIFU::~MainWindowLIFU()
{
    delete ui;
}

QLabel *MainWindowLIFU::GetConnectLabel()
{
    return ui->lblState;
}

QLabel *MainWindowLIFU::GetEmitLabel()
{
    return ui->lblEmitState;
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
    OnClickCancel();
}

void MainWindowLIFU::InitEvent()
{
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
    m_DataManager->m_CurrentProfileLIFU = m_DataManager->m_ProfileListLIFU[loadIndex];
    OnClickCancel();
}

void MainWindowLIFU::OnClickOff()
{
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
}

void MainWindowLIFU::OnClickOn()
{
    SetEmitState(EmitState::ON);
    UpdateBtnState();
}

void MainWindowLIFU::OnClickSave()
{
    int saveType = 0;
    SaveDialog *dialog = new SaveDialog(saveType, this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
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
            ui->btnEdit->setVisible(true);
            ui->btnLoad->setVisible(true);
            ui->btnOn->setVisible(true);
        }
    }
}
