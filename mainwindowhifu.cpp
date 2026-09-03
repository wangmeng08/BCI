#include "mainwindowhifu.h"
#include "ui_mainwindowhifu.h"
#include "constvalue.h"
#include "datamanager.h"
#include "eventmanager.h"
#include "hifupulsetrainadvance.h"
#include "logmanager.h"
#include "messageinfo.h"
#include "option.h"
#include "profileload.h"
#include "savedialog.h"

#include <qtabbar.h>

namespace {

bool SameDouble(double left, double right)
{
    return qAbs(left - right) < 0.000001;
}

uint32_t EncodePdUs(int periodMs, int dutyPercent)
{
    const int boundedDutyPercent = qBound(0, dutyPercent, 100);
    return static_cast<uint32_t>(qMax(1, periodMs) * boundedDutyPercent * 10);
}

bool IsOnlyTimerChanged(const Profile *profile, const Profile *current)
{
    if (!profile || !current || profile->timer == current->timer)
        return false;
    if (profile->profileName != current->profileName ||
        !SameDouble(profile->isppa, current->isppa) ||
        !SameDouble(profile->rip, current->rip) ||
        profile->period != current->period ||
        profile->dc != current->dc ||
        !SameDouble(profile->depth, current->depth) ||
        !SameDouble(profile->freq, current->freq) ||
        !SameDouble(profile->temp, current->temp) ||
        !SameDouble(profile->voltage, current->voltage) ||
        !SameDouble(profile->burstLen, current->burstLen) ||
        profile->infoList.size() != current->infoList.size()) {
        return false;
    }
    for (int i = 0; i < profile->infoList.size(); ++i) {
        const DetailInfo *left = profile->infoList[i];
        const DetailInfo *right = current->infoList[i];
        if (!left || !right)
            return false;
        if (!SameDouble(left->delay, right->delay) ||
            !SameDouble(left->freq, right->freq) ||
            !SameDouble(left->hcd, right->hcd)) {
            return false;
        }
    }
    return true;
}

}

MainWindowHIFU::MainWindowHIFU(QWidget *parent)
    : BaseWindow(parent)
    , ui(new Ui::MainWindowHIFU)
{
    ui->setupUi(this);
    emit EventManager::GetInstance()->writeLog(LogType::INFO, "HIFU start");

    InitData();
    InitEvent();
    SyncConnectStateFromSerial();
    SetEmitState(EmitState::IDLE);
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
    SendCommandSystemHostConnectStatus(HostControlMode::REMOTE);
    SendCommandSystemStop();
    QSharedPointer<Profile> profile = m_DataManager->m_CurrentProfile;
    if (profile.isNull())
        return;

    const int periodMs = qMax(1, profile->period);
    const int dutyPercent = qBound(0, profile->dc, 100);
    const uint32_t pdUs = EncodePdUs(periodMs, dutyPercent);

    QVector<uint32_t> delays;
    delays.reserve(profile->infoList.size());
    for (const DetailInfo *info : qAsConst(profile->infoList)) {
        if (!info)
            continue;
        delays.append(static_cast<uint32_t>(qMax(0.0, info->delay)));
    }

    WriteCommLog(QStringLiteral(
                     "[ON] HIFU parameters: HVOut=%1 V, Frequency=%2 kHz, "
                     "PRI=%3 ms, DC=%4%, PD=%5 us, Timer=%6 ms, "
                     "ChannelDelays count=%7")
                 .arg(profile->voltage)
                 .arg(profile->freq)
                 .arg(periodMs)
                 .arg(dutyPercent)
                 .arg(pdUs)
                 .arg(profile->timer)
                 .arg(delays.size()));

    SendCommandSetHvout(profile->voltage);
    SendCommandSetIsppa(profile->isppa);
    SendCommandSetChannelSwitch(delays.size());
    //SendCommandSetFrequency(static_cast<uint32_t>(qRound(profile->freq)));
    //SendCommandSetChannelDelay(delays);
    SendCommandSetPri(static_cast<uint32_t>(periodMs));
    SendCommandSetPD(pdUs);
    SendCommandSetEmitTime(static_cast<uint32_t>(qMax(0, profile->timer)));
    SendCommandSystemTriggerModel();
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
    m_PulseTrainAdvance = new HifuPulseTrainAdvance(this);
    ui->verticalLayout_4->insertWidget(0, m_PulseTrainAdvance, 1);
    ui->widget_5->hide();
    m_PulseTrainTimer = new QTimer(this);
    m_PulseTrainTimer->setSingleShot(false);
    m_PulseTrainDisplayTimer = new QTimer(this);
    m_PulseTrainDisplayTimer->setInterval(m_timerIntervalMs);
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
    connect(m_PulseTrainTimer, &QTimer::timeout,
            this, &MainWindowHIFU::PulseTrainTick);
    connect(m_PulseTrainDisplayTimer, &QTimer::timeout,
            this, &MainWindowHIFU::PulseTrainDisplayTick);

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
    if (m_PulseTrainAdvance)
        m_PulseTrainAdvance->LoadFromProfile(m_DataManager->m_CurrentProfile.get());
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

void MainWindowHIFU::OnClickOff()
{
    if (m_PulseTrainActive) {
        StopPulseTrain(true);
        return;
    }
    BaseWindow::OnClickOff();
}

void MainWindowHIFU::OnClickOn()
{
    const bool isAdvancePage = ui->tabWidget->currentIndex() == 1;
    if (isAdvancePage && m_PulseTrainAdvance) {
        m_IsInAdvance = true;
        if (m_PulseTrainActive)
            return;
        if (!IsSerialAvailable()) {
            WriteCommLog(QStringLiteral("[MCU] serial port unavailable, opening serial port only"));
            emit serialPortClose();
            emit serialPortOpen();
            return;
        }
        if (!IsSerialOpen() || m_ConnectState == ConnectState::DISCONNECT) {
            WriteCommLog(QStringLiteral("[MCU] serial disconnected, opening serial port only"));
            emit serialPortOpen();
            return;
        }
        StartPulseTrain();
        return;
    }

    BaseWindow::OnClickOn();
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
    profile->burstLen = ui->lblBurstLen->text().toDouble();
    if(m_IsInAdvance)
    {
        ApplyPulseTrainAdvanceToProfile(profile.get());
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
    if (IsOnlyTimerChanged(profile.get(), m_DataManager->m_CurrentProfile.get())) {
        if (m_DataManager->SaveInfoToCurrentProfile(profile)) {
            SetEditMode(false);
            OnClickCancel();
            OnCurrentProfileChange(pre, m_DataManager->m_CurrentProfile);
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
    if (curr.isNull())
        return;

    if(prev.isNull() || prev->voltage != curr->voltage)
    {
        SendCommandSetHvout(curr->voltage);
    }
    if(prev.isNull() || !SameDouble(prev->isppa, curr->isppa))
    {
        SendCommandSetIsppa(curr->isppa);
    }
    if(prev.isNull() || prev->freq != curr->freq)
    {
        //SendCommandSetFrequency(static_cast<uint32_t>(qRound(curr->freq)));
    }
    if(prev.isNull() || prev->period != curr->period)
    {
        SendCommandSetPri(curr->period);
    }
    if(prev.isNull() || prev->period != curr->period || prev->dc != curr->dc)
    {
        const int periodMs = qMax(1, curr->period);
        SendCommandSetPD(EncodePdUs(periodMs, curr->dc));
    }
    if(prev.isNull() || prev->timer != curr->timer)
    {
        SendCommandSetEmitTime(curr->timer);
    }
    if(prev.isNull())
    {
        SendCommandSetChannelSwitch(curr->infoList.size());
        QVector<uint32_t> delays;
        delays.reserve(curr->infoList.size());
        for (const DetailInfo *info : qAsConst(curr->infoList)) {
            if (!info)
                continue;
            delays.append(static_cast<uint32_t>(qMax(0.0, info->delay)));
        }
        //SendCommandSetChannelDelay(delays);
        SendCommandSystemTriggerModel();
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
    Q_UNUSED(isInit);

}

void MainWindowHIFU::ApplyPulseTrainAdvanceToProfile(Profile *profile) const
{
    if (!profile || !m_PulseTrainAdvance)
        return;

    profile->isppa = m_PulseTrainAdvance->isppa();
    profile->freq = m_PulseTrainAdvance->frequencyKhz();
    profile->period = m_PulseTrainAdvance->priMs();
    profile->dc = m_PulseTrainAdvance->dutyCyclePercent();
    profile->timer = m_PulseTrainAdvance->pulseTrainDurationMs();
    profile->burstLen = m_PulseTrainAdvance->pulseDurationMs();
}

void MainWindowHIFU::StartPulseTrain()
{
    QSharedPointer<Profile> prev = QSharedPointer<Profile>::create();
    prev->CopyInfo(m_DataManager->m_CurrentProfile.get());
    QSharedPointer<Profile> next = QSharedPointer<Profile>::create();
    next->CopyInfo(m_DataManager->m_CurrentProfile.get());
    ApplyPulseTrainAdvanceToProfile(next.get());
    m_DataManager->m_CurrentProfile = next;
    OnCurrentProfileChange(prev, m_DataManager->m_CurrentProfile);

    m_PulseTrainIntervalMs = qMax(1, qRound(m_PulseTrainAdvance->stimIntervalS() * 1000.0));
    m_PulseTrainPulseTimerMs = qMax(1, m_PulseTrainAdvance->pulseTrainDurationMs());
    m_PulseTrainRemainingMs = qMax(1, qRound(m_PulseTrainAdvance->stimDurationS() * 1000.0));
    m_PulseTrainTotalCount = PulseTrainCount();
    m_PulseTrainSentCount = 0;
    m_PulseTrainActive = true;
    m_EmitStartPending = false;

    WriteCommLog(QStringLiteral(
                     "[PULSE TRAIN] start: stimDuration=%1 ms, interval=%2 ms, "
                     "pulseTimer=%3 ms, count=%4")
                 .arg(m_PulseTrainRemainingMs)
                 .arg(m_PulseTrainIntervalMs)
                 .arg(m_PulseTrainPulseTimerMs)
                 .arg(m_PulseTrainTotalCount));

    SetEmitState(EmitState::ON);
    UpdateBtnState();
    m_CurrentTime = m_PulseTrainRemainingMs;
    SetTimerInfo();
    m_PulseTrainDisplayTimer->start();
    m_PulseTrainTimer->setInterval(m_PulseTrainIntervalMs);
    PulseTrainTick();
    if (m_PulseTrainActive)
        m_PulseTrainTimer->start();
}

void MainWindowHIFU::StopPulseTrain(bool sendStopCommand)
{
    if (m_PulseTrainTimer)
        m_PulseTrainTimer->stop();
    if (m_PulseTrainDisplayTimer)
        m_PulseTrainDisplayTimer->stop();
    const bool wasActive = m_PulseTrainActive;
    m_PulseTrainActive = false;
    m_PulseTrainRemainingMs = 0;
    m_CurrentTime = 0;
    SetTimerInfo();
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
    if (sendStopCommand)
        SendCommandSystemStop();
    if (wasActive)
        WriteCommLog(QStringLiteral("[PULSE TRAIN] stop"));
}

void MainWindowHIFU::PulseTrainTick()
{
    if (!m_PulseTrainActive)
        return;
    if (m_PulseTrainSentCount >= m_PulseTrainTotalCount) {
        StopPulseTrain(true);
        return;
    }

    ++m_PulseTrainSentCount;
    WriteCommLog(QStringLiteral("[PULSE TRAIN] pulse %1/%2, timer=%3 ms")
                 .arg(m_PulseTrainSentCount)
                 .arg(m_PulseTrainTotalCount)
                 .arg(m_PulseTrainPulseTimerMs));
    SendCommandSetEmitTime(static_cast<uint32_t>(m_PulseTrainPulseTimerMs));
    SendCommandSystemEmit();

    if (m_PulseTrainSentCount >= m_PulseTrainTotalCount)
        m_PulseTrainTimer->stop();
}

void MainWindowHIFU::PulseTrainDisplayTick()
{
    if (!m_PulseTrainActive)
        return;

    m_PulseTrainRemainingMs = qMax(0, m_PulseTrainRemainingMs - m_timerIntervalMs);
    m_CurrentTime = m_PulseTrainRemainingMs;
    SetTimerInfo();
    if (m_PulseTrainRemainingMs <= 0)
        StopPulseTrain(true);
}

int MainWindowHIFU::PulseTrainCount() const
{
    if (!m_PulseTrainAdvance)
        return 1;

    const int stimDurationMs = qMax(1, qRound(m_PulseTrainAdvance->stimDurationS() * 1000.0));
    const int intervalMs = qMax(1, qRound(m_PulseTrainAdvance->stimIntervalS() * 1000.0));
    const int pulseTimerMs = qMax(1, m_PulseTrainAdvance->pulseTrainDurationMs());
    if (stimDurationMs <= pulseTimerMs)
        return 1;
    return ((stimDurationMs - pulseTimerMs) / intervalMs) + 1;
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
    if (m_PulseTrainAdvance)
        m_PulseTrainAdvance->SetEditMode(isEdit);
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

