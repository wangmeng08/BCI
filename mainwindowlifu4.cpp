#include "mainwindowlifu4.h"
#include "constvalue.h"
#include "ui_mainwindowlifu.h"
#include "profileload.h"
#include "savedialog.h"
#include <QListWidgetItem>

namespace {

bool SameDouble(double left, double right)
{
    return qAbs(left - right) < 0.000001;
}

bool IsOnlyTimerChanged(const ProfileLIFU4 *profile, const ProfileLIFU4 *current)
{
    if (!profile || !current || profile->timer == current->timer)
        return false;
    if (profile->profileName != current->profileName ||
        !SameDouble(profile->dutyc, current->dutyc) ||
        profile->period != current->period ||
        !SameDouble(profile->temp, current->temp) ||
        !SameDouble(profile->voltage, current->voltage)) {
        return false;
    }
    for (int i = 0; i < ProfileLIFU4::ValueCount; ++i) {
        if (profile->values[i] != current->values[i])
            return false;
    }
    return true;
}

uint32_t EncodePdUs(int periodMs, double dutyPercent)
{
    const double boundedDutyPercent = qBound(0.0, dutyPercent, 100.0);
    return static_cast<uint32_t>(qRound(qMax(1, periodMs) * boundedDutyPercent * 10.0));
}

}

MainWindowLIFU4::MainWindowLIFU4(QWidget *parent) :
    BaseWindow(parent),
    ui(new Ui::MainWindowLIFU)
{
    ui->setupUi(this);
    InitData();
    InitEvent();
    SyncConnectStateFromSerial();
    SetEmitState(EmitState::IDLE);
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

QLabel *MainWindowLIFU4::GetStateIcon()
{
    return ui->lblStateIcon;
}

void MainWindowLIFU4::SendInitCommand()
{
    SendCommandSystemModel();
    const double voltage = ui->lblVoltage->text().toDouble();
    const double dutyCycle = ui->lblDutyc->text().toDouble();
    const int periodMs = ui->lblPeriod->text().toInt();
    const int timerMs = ui->lblTimer->text().toInt();

    WriteCommLog(QStringLiteral(
                     "[ON] UI parameters: DutyC=%1%%, HVOut=%2 V, "
                     "PRI=%3 ms, Timer=%4 ms")
                 .arg(dutyCycle).arg(voltage).arg(periodMs).arg(timerMs));

    SendCommandSetHvout(voltage);
    SendCommandSetChannelSwitch(ProfileLIFU4::ValueCount);

    QVector<uint32_t> delays;
    delays.reserve(ProfileLIFU4::ValueCount);
    for (TXItem *item : m_VectorItem)
        delays.append(static_cast<uint32_t>(qMax(0, item->GetInfo())));
    WriteCommLog(QStringLiteral(
                     "[ON] ChannelDelays count=%1, first=%2, last=%3")
                 .arg(delays.size())
                 .arg(delays.isEmpty() ? 0 : delays.first())
                 .arg(delays.isEmpty() ? 0 : delays.last()));
    //SendCommandSetChannelDelay(delays);

    SendCommandSetPri(static_cast<uint32_t>(qMax(0, periodMs)));
    SendCommandSetPD(EncodePdUs(periodMs, dutyCycle));
    SendCommandSetEmitTime(static_cast<uint32_t>(qMax(0, timerMs)));
}

void MainWindowLIFU4::SetTimerInfo()
{
    ui->lblTimer->setText(QString("%1").arg(m_CurrentTime));
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
    connect(ui->btnCancel, &QPushButton::clicked, this, &MainWindowLIFU4::OnClickCancel);
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

    if(loadIndex < ConstValue::GetInstance()->DeleteLimit)
    {
        m_DataManager->m_CurrentProfileLIFU4 = m_DataManager->m_ProfileListLIFU4[loadIndex];
        OnClickCancel();
    }
    else
    {
        int index = loadIndex - ConstValue::GetInstance()->DeleteLimit;
        if(m_DataManager->m_CurrentProfileLIFU4 == m_DataManager->m_ProfileListLIFU4[index])
        {
            m_DataManager->m_CurrentProfileLIFU4 = m_DataManager->m_ProfileListLIFU4[0];
            OnClickCancel();
        }
        m_DataManager->DeleteProfileLIFU4(m_DataManager->m_ProfileListLIFU4[index]);
    }
}

void MainWindowLIFU4::OnClickSave()
{
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
    if (IsOnlyTimerChanged(profile.get(), m_DataManager->m_CurrentProfileLIFU4.get())) {
        if (m_DataManager->SaveInfoToCurrentProfileLIFU4(profile)) {
            SetEditMode(false);
            OnClickCancel();
            SendCommandSetEmitTime(static_cast<uint32_t>(qMax(0, profile->timer)));
        }
        return;
    }

    int saveType = 0;
    QString saveName = "";
    SaveDialog *dialog = new SaveDialog(saveType, saveName, this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    dialog->exec();
    delete dialog;
    bool res = true;
    switch(saveType)
    {
    case(0):
        res = m_DataManager->SaveInfoToCurrentProfileLIFU4(profile);
        break;
    case(1):
        profile->profileName = saveName;
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
