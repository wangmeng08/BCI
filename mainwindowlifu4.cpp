#include "mainwindowlifu4.h"
#include "ui_mainwindowlifu4.h"
#include "profileload.h"
#include "savedialog.h"
#include <QListWidgetItem>

MainWindowLIFU4::MainWindowLIFU4(QWidget *parent) :
    BaseWindow(parent),
    ui(new Ui::MainWindowLIFU4)
{
    ui->setupUi(this);
    InitData();
    InitEvent();
}

MainWindowLIFU4::~MainWindowLIFU4()
{
    delete ui;
}

void MainWindowLIFU4::InitProfileData()
{
    OnClickCancel();
}

QLabel *MainWindowLIFU4::GetConnectLabel()
{
    return ui->lblState;
}

QLabel *MainWindowLIFU4::GetEmitLabel()
{
    return ui->lblEmitState;
}

void MainWindowLIFU4::InitData()
{

    ui->listWidget->setFlow(QListView::LeftToRight);
    ui->listWidget->setWrapping(true);
    ui->listWidget->setResizeMode(QListView::Adjust);
    for(int i=0; i<ProfileLIFU4::ValueCount; i++)
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

void MainWindowLIFU4::InitEvent()
{
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindowLIFU4::OnClickEdit);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindowLIFU4::OnClickLoad);
    connect(ui->btnOff, &QPushButton::clicked, this, &MainWindowLIFU4::OnClickOff);
    connect(ui->btnOn, &QPushButton::clicked, this, &MainWindowLIFU4::OnClickOn);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindowLIFU4::OnClickSave);
}

void MainWindowLIFU4::OnClickCancel()
{
    for(int i=0; i<ProfileLIFU4::ValueCount; i++)
    {
        m_VectorItem[i]->SetInfo(m_DataManager->m_CurrentProfileLIFU4->values[i]);
    }
    ui->lblDutyc->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU4->dutyc));
    ui->lblPeriod->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU4->period));
    ui->lblTemp->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU4->temp));
    ui->lblTimer->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU4->timer));
    ui->lblVoltage->setText(QString("%1").arg(m_DataManager->m_CurrentProfileLIFU4->voltage));
    ui->lblName->setText(m_DataManager->m_CurrentProfileLIFU4->profileName);
}

void MainWindowLIFU4::OnClickEdit()
{
    m_IsInEdit = true;
    SetEditMode(m_IsInEdit);
}

void MainWindowLIFU4::OnClickLoad()
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
    m_DataManager->m_CurrentProfileLIFU4 = m_DataManager->m_ProfileListLIFU4[loadIndex];
    OnClickCancel();
}

void MainWindowLIFU4::OnClickOff()
{
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
}

void MainWindowLIFU4::OnClickOn()
{
    SetEmitState(EmitState::ON);
    UpdateBtnState();
}

void MainWindowLIFU4::OnClickSave()
{
    int saveType = 0;
    SaveDialog *dialog = new SaveDialog(saveType, this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
    QSharedPointer<ProfileLIFU4> profile = QSharedPointer<ProfileLIFU4>::create();
    profile->profileName = ui->lblName->text();
    profile->dutyc = ui->lblDutyc->text().toDouble();
    profile->period = ui->lblPeriod->text().toInt();
    profile->timer = ui->lblTimer->text().toInt();
    profile->temp = ui->lblTemp->text().toDouble();
    profile->voltage = ui->lblVoltage->text().toDouble();
    for(int i=0; i<ProfileLIFU4::ValueCount; i++)
    {
        profile->values[i] = m_VectorItem[i]->GetInfo();
    }
    bool res = true;
    switch(saveType)
    {
    case(0):
        res = m_DataManager->SaveInfoToCurrentProfileLIFU4(profile);
        break;
    case(1):
        res = m_DataManager->SaveInfoToNewProfileLIFU4(profile);
        break;
    case(2):
        res = m_DataManager->SaveInfoToDefaultProfileLIFU4(profile);
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

void MainWindowLIFU4::SetEditMode(bool isEdit)
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

void MainWindowLIFU4::UpdateBtnState()
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
