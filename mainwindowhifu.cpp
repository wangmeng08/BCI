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
#include "treatmentrecorder.h"

#include <qtabbar.h>
#include <QGridLayout>
#include <QLabel>

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
        profile->stimDurationMs != current->stimDurationMs ||
        profile->stimIntervalMs != current->stimIntervalMs ||
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
    QSharedPointer<Profile> profile = m_DataManager->m_CurrentProfile;
    if (profile.isNull())
        return;

    const int periodMs = qMax(1, profile->period);
    const int dutyPercent = qBound(0, profile->dc, 100);
    const uint32_t pdUs = EncodePdUs(periodMs, dutyPercent);

    WriteCommLog(QStringLiteral(
                     "[ON] HIFU parameters: Frequency=%1 kHz, "
                     "PRI=%2 ms, DC=%3%, PD=%4 us, Timer=%5 ms")
                 .arg(profile->freq)
                 .arg(periodMs)
                 .arg(dutyPercent)
                 .arg(pdUs)
                 .arg(profile->timer));

    // The current HIFU firmware rejects HVOut and channel-switch writes.
    // Keep Local On aligned with the working Advance pulse-train path.
    SendCommandSetIsppa(profile->isppa);
    //SendCommandSetFrequency(static_cast<uint32_t>(qRound(profile->freq)));
    SendCommandSetPri(static_cast<uint32_t>(periodMs));
    SendCommandSetPD(pdUs);
    SendCommandSetEmitTime(static_cast<uint32_t>(qMax(0, profile->timer)));
}

void MainWindowHIFU::SetTimerInfo()
{
    ui->lblTimer->setText(QString("%1").arg(m_CurrentTime));
    ui->lblTimer2->setText(QString("%1").arg(m_CurrentTime));
}

void MainWindowHIFU::OnEmitTimerExpired()
{
    if (m_PulseTrainActive || m_PulseTrainPaused) {
        SetTimerInfo();
        return;
    }

    const int savedTimer = m_DataManager->GetEmitTime();
    ui->lblTimer->setText(QString("%1").arg(savedTimer));
    ui->lblTimer2->setText(QString("%1").arg(savedTimer));
}

void MainWindowHIFU::OnSonicStarted(uint8_t addr)
{
    Q_UNUSED(addr);

    const bool isPulseTrain = m_PulseTrainActive || m_PulseTrainPaused;
    if (isPulseTrain) {
        m_PulseTrainStarted = true;
        return;
    }

    m_LocalTreatmentStarted = true;
    m_LocalTreatmentRecorded = false;
}

void MainWindowHIFU::OnSonicFinished()
{
    if (m_PulseTrainActive || m_PulseTrainPaused)
        return;

    RecordLocalTreatmentIfNeeded();
    m_EmitStartPending = false;
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
    EmitTimerStop();
    m_CurrentTime = m_DataManager->GetEmitTime();
    SetTimerInfo();
}

void MainWindowHIFU::OnSonicStopped()
{
    if (!m_PulseTrainActive && !m_PulseTrainPaused) {
        RecordLocalTreatmentIfNeeded();
        m_CurrentTime = m_DataManager->GetEmitTime();
        SetTimerInfo();
    }
}

void MainWindowHIFU::InitData()
{
    ui->tabWidget->tabBar()->hide();
    ui->tabWidget->setCurrentIndex(0);
    RebuildLocalLayout();
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
    connect(ui->lblPeriod, &QLineEdit::editingFinished,
            this, &MainWindowHIFU::UpdateLocalDerivedValues);
    connect(ui->lblDC, &QLineEdit::editingFinished,
            this, &MainWindowHIFU::UpdateLocalDerivedValues);
    connect(ui->lblIsppa, &QLineEdit::editingFinished,
            this, &MainWindowHIFU::UpdateLocalDerivedValues);
    connect(m_PulseTrainTimer, &QTimer::timeout,
            this, &MainWindowHIFU::PulseTrainTick);
    connect(m_PulseTrainDisplayTimer, &QTimer::timeout,
            this, &MainWindowHIFU::PulseTrainDisplayTick);
    connect(m_PulseTrainAdvance, &HifuPulseTrainAdvance::startPauseRequested,
            this, &MainWindowHIFU::OnPulseTrainStartPause);
    connect(m_PulseTrainAdvance, &HifuPulseTrainAdvance::stopRequested,
            this, &MainWindowHIFU::OnClickOff);

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
    UpdateLocalDerivedValues();
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
    if (m_PulseTrainActive || m_PulseTrainPaused)
        StopPulseTrain(true);
    ui->tabWidget->setCurrentIndex(0);
    m_IsInAdvance = false;
    OnClickCancel();
    SetAdvanceBtnState();
}

void MainWindowHIFU::OnClickOff()
{
    if (m_PulseTrainActive || m_PulseTrainPaused) {
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
        if (m_PulseTrainPaused) {
            ResumePulseTrain();
            return;
        }
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
    profile->CopyInfo(m_DataManager->m_CurrentProfile.get());
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
        SendCommandSystemTriggerModel();
    }
}

void MainWindowHIFU::OnModeAdvanceRFChange()
{
}

void MainWindowHIFU::OnModePowerLimitChange()
{
    ui->lblWarning->setVisible(!m_DataManager->m_IsUsePowerLimit);
    const QString qss = m_DataManager->m_IsUsePowerLimit
            ? QStringLiteral("color: rgb(255,255,255); font-size: 26px; background: transparent; border: none;")
            : QStringLiteral("color: red; font-size: 26px; background: transparent; border: none;");
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
    profile->stimDurationMs = qMax(0, qRound(m_PulseTrainAdvance->stimDurationS() * 1000.0));
    profile->stimIntervalMs = qMax(0, qRound(m_PulseTrainAdvance->stimIntervalS() * 1000.0));
}

void MainWindowHIFU::StartPulseTrain()
{
    QSharedPointer<Profile> prev = QSharedPointer<Profile>::create();
    prev->CopyInfo(m_DataManager->m_CurrentProfile.get());
    QSharedPointer<Profile> next = QSharedPointer<Profile>::create();
    next->CopyInfo(m_DataManager->m_CurrentProfile.get());
    ApplyPulseTrainAdvanceToProfile(next.get());
    m_PulseTrainProfile = next;
    OnCurrentProfileChange(prev, m_PulseTrainProfile);

    m_PulseTrainPulseTimerMs = qMax(1, m_PulseTrainAdvance->pulseTrainDurationMs());
    m_PulseTrainIntervalMs = PulseTrainPeriodMs();
    m_PulseTrainTotalDurationMs = qMax(1, m_PulseTrainProfile->stimDurationMs);
    m_PulseTrainRemainingMs = m_PulseTrainTotalDurationMs;
    m_PulseTrainElapsedBeforeResumeMs = 0;
    m_PulseTrainTotalCount = PulseTrainCount();
    m_PulseTrainSentCount = 0;
    m_PulseTrainActive = true;
    m_PulseTrainPaused = false;
    m_PulseTrainRecorded = false;
    m_PulseTrainStarted = false;
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
    m_PulseTrainAdvance->SetPulseTrainRunning(true);
    m_CurrentTime = m_PulseTrainRemainingMs;
    SetTimerInfo();
    m_PulseTrainAdvance->UpdateProgress(0, m_PulseTrainRemainingMs,
                                        m_PulseTrainTotalDurationMs);
    m_PulseTrainElapsedTimer.restart();
    m_PulseTrainDisplayTimer->start();
    m_PulseTrainTimer->setInterval(m_PulseTrainIntervalMs);
    PulseTrainTick();
    if (m_PulseTrainActive)
        m_PulseTrainTimer->start();
}

void MainWindowHIFU::StopPulseTrain(bool sendStopCommand)
{
    if (m_PulseTrainActive)
        UpdatePulseTrainProgressFromClock();
    if (m_PulseTrainTimer)
        m_PulseTrainTimer->stop();
    if (m_PulseTrainDisplayTimer)
        m_PulseTrainDisplayTimer->stop();
    const bool hadTrain = m_PulseTrainActive || m_PulseTrainPaused;
    const int totalMs = qMax(0, m_PulseTrainTotalDurationMs);
    const int remainingMs = qBound(0, m_PulseTrainRemainingMs, totalMs);
    const int elapsedMs = qMax(0, totalMs - remainingMs);
    m_PulseTrainActive = false;
    m_PulseTrainPaused = false;
    m_PulseTrainElapsedBeforeResumeMs = 0;
    RecordPulseTrainTreatmentIfNeeded(elapsedMs);
    m_PulseTrainStarted = false;
    m_CurrentTime = remainingMs;
    SetTimerInfo();
    if (m_PulseTrainAdvance)
        m_PulseTrainAdvance->ResetProgress();
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
    if (m_PulseTrainAdvance)
        m_PulseTrainAdvance->SetPulseTrainRunning(false);
    if (sendStopCommand)
        SendCommandSystemStop();
    if (hadTrain)
        WriteCommLog(QStringLiteral("[PULSE TRAIN] stop"));
}

void MainWindowHIFU::RecordLocalTreatmentIfNeeded()
{
    if (!m_LocalTreatmentStarted || m_LocalTreatmentRecorded)
        return;

    const Profile *profile = m_DataManager->m_CurrentProfile.get();
    if (!profile)
        return;

    const int plannedMs = qMax(0, profile->timer);
    const int actualMs = qBound(0, plannedMs - m_CurrentTime, plannedMs);
    TreatmentRecorder::GetInstance()->RecordHifuTreatment(
                m_DataManager->m_CurrentPatient.get(),
                profile,
                QStringLiteral("Local"),
                actualMs,
                1);
    m_LocalTreatmentRecorded = true;
    m_LocalTreatmentStarted = false;
}

void MainWindowHIFU::RecordPulseTrainTreatmentIfNeeded(int actualTreatmentMs)
{
    if (!m_PulseTrainStarted || m_PulseTrainRecorded)
        return;

    const Profile *profile = m_PulseTrainProfile.isNull()
            ? m_DataManager->m_CurrentProfile.get()
            : m_PulseTrainProfile.get();
    TreatmentRecorder::GetInstance()->RecordHifuTreatment(
                m_DataManager->m_CurrentPatient.get(),
                profile,
                QStringLiteral("Advance"),
                qMax(0, actualTreatmentMs),
                qMax(1, m_PulseTrainSentCount),
                m_PulseTrainPulseTimerMs,
                m_PulseTrainIntervalMs,
                m_PulseTrainTotalDurationMs);
    m_PulseTrainRecorded = true;
}

void MainWindowHIFU::OnPulseTrainStartPause()
{
    if (m_PulseTrainActive) {
        PausePulseTrain();
        return;
    }
    if (m_PulseTrainPaused) {
        ResumePulseTrain();
        return;
    }
    OnClickOn();
}

void MainWindowHIFU::PausePulseTrain()
{
    if (!m_PulseTrainActive)
        return;

    UpdatePulseTrainProgressFromClock();
    if (m_PulseTrainTimer)
        m_PulseTrainTimer->stop();
    if (m_PulseTrainDisplayTimer)
        m_PulseTrainDisplayTimer->stop();
    m_PulseTrainActive = false;
    m_PulseTrainPaused = true;
    m_PulseTrainElapsedBeforeResumeMs =
            qBound(0,
                   m_PulseTrainTotalDurationMs - m_PulseTrainRemainingMs,
                   m_PulseTrainTotalDurationMs);
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
    if (m_PulseTrainAdvance)
        m_PulseTrainAdvance->SetPulseTrainRunning(false, true);
    SendCommandSystemStop();
    WriteCommLog(QStringLiteral("[PULSE TRAIN] pause"));
}

void MainWindowHIFU::ResumePulseTrain()
{
    if (!m_PulseTrainPaused || m_PulseTrainRemainingMs <= 0)
        return;

    m_PulseTrainPaused = false;
    m_PulseTrainActive = true;
    SetEmitState(EmitState::ON);
    UpdateBtnState();
    if (m_PulseTrainAdvance)
        m_PulseTrainAdvance->SetPulseTrainRunning(true);
    if (m_PulseTrainDisplayTimer)
        m_PulseTrainDisplayTimer->start();
    if (m_PulseTrainTimer)
        m_PulseTrainTimer->setInterval(m_PulseTrainIntervalMs);
    m_PulseTrainElapsedTimer.restart();
    PulseTrainTick();
    if (m_PulseTrainActive && m_PulseTrainTimer)
        m_PulseTrainTimer->start();
    WriteCommLog(QStringLiteral("[PULSE TRAIN] resume"));
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

    UpdatePulseTrainProgressFromClock();
    if (m_PulseTrainRemainingMs <= 0)
        StopPulseTrain(true);
}

int MainWindowHIFU::PulseTrainCount() const
{
    if (!m_PulseTrainAdvance)
        return 1;

    const int stimDurationMs = m_PulseTrainProfile.isNull()
            ? qMax(1, qRound(m_PulseTrainAdvance->stimDurationS() * 1000.0))
            : qMax(1, m_PulseTrainProfile->stimDurationMs);
    const int pulseTimerMs = qMax(1, m_PulseTrainAdvance->pulseTrainDurationMs());
    const int intervalMs = PulseTrainPeriodMs();
    if (stimDurationMs <= pulseTimerMs)
        return 1;
    return ((stimDurationMs - pulseTimerMs) / intervalMs) + 1;
}

int MainWindowHIFU::PulseTrainPeriodMs() const
{
    if (!m_PulseTrainAdvance)
        return 1;

    const int pulseTimerMs = qMax(1, m_PulseTrainAdvance->pulseTrainDurationMs());
    const int stimIntervalMs = m_PulseTrainProfile.isNull()
            ? qMax(0, qRound(m_PulseTrainAdvance->stimIntervalS() * 1000.0))
            : qMax(0, m_PulseTrainProfile->stimIntervalMs);
    return stimIntervalMs <= 0 ? pulseTimerMs : stimIntervalMs;
}

void MainWindowHIFU::UpdatePulseTrainProgressFromClock()
{
    const qint64 elapsedSinceResume = m_PulseTrainElapsedTimer.isValid()
            ? m_PulseTrainElapsedTimer.elapsed()
            : 0;
    const int elapsedMs = qBound(0,
                                 m_PulseTrainElapsedBeforeResumeMs
                                 + static_cast<int>(elapsedSinceResume),
                                 m_PulseTrainTotalDurationMs);
    m_PulseTrainRemainingMs = qMax(0, m_PulseTrainTotalDurationMs - elapsedMs);
    m_CurrentTime = m_PulseTrainRemainingMs;
    SetTimerInfo();
    if (m_PulseTrainAdvance) {
        m_PulseTrainAdvance->UpdateProgress(elapsedMs, m_PulseTrainRemainingMs,
                                            m_PulseTrainTotalDurationMs);
    }
}

void MainWindowHIFU::RebuildLocalLayout()
{
    QGridLayout *layout = ui->gridLayout;
    while (QLayoutItem *item = layout->takeAt(0))
        delete item;

    if (!m_LocalPdTitle)
        m_LocalPdTitle = new QLabel(this);
    if (!m_LocalPdValue)
        m_LocalPdValue = new QLabel(this);

    ui->label_2->setText(QStringLiteral("Freq(kHz):"));
    ui->label_12->setTextFormat(Qt::RichText);
    ui->label_5->setTextFormat(Qt::RichText);
    ui->label_12->setText(QStringLiteral("Isppa(W/cm<sup>2</sup>):"));
    ui->label_5->setText(QStringLiteral("Ispta(W/cm<sup>2</sup>):"));
    ui->label->setText(QStringLiteral("Power(W):"));
    ui->label_4->setText(QStringLiteral("XDC:"));
    ui->label_11->setText(QStringLiteral("Depth(mm):"));
    ui->label_9->setText(QStringLiteral("Voltage(V):"));
    ui->label_3->setText(QStringLiteral("Temp(C):"));
    ui->label_15->setText(QStringLiteral("PRI(ms):"));
    m_LocalPdTitle->setText(QStringLiteral("PD(ms):"));
    ui->label_14->setText(QStringLiteral("DC(%):"));
    ui->label_8->setText(QStringLiteral("Profile:"));
    ui->label_13->setText(QStringLiteral("Timer(ms):"));

    const QString labelStyle = QStringLiteral("color: rgba(255,255,255,153); font-size: 24px;");
    const QString valueStyle = QStringLiteral("color: rgb(255,255,255); font-size: 26px; background: transparent; border: none;");
    const QFont labelFont(QStringLiteral("Alibaba PuHuiTi 2.0 55 Regular"), 24);
    const QFont valueFont(QStringLiteral("Alibaba PuHuiTi 2.0 55 Regular"), 26);
    const QList<QLabel *> labels = {
        ui->label_2, ui->label_12, ui->label_5, ui->label,
        ui->label_4, ui->label_11, ui->label_9, ui->label_3,
        ui->label_15, m_LocalPdTitle, ui->label_14, ui->label_8,
        ui->label_13
    };
    for (QLabel *label : labels) {
        label->setFont(labelFont);
        label->setStyleSheet(labelStyle);
        label->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    }

    const QList<QWidget *> values = {
        ui->lblFreq, ui->lblIsppa, ui->lblIspta, ui->lblPower,
        ui->label_6, ui->lblDepth, ui->lblVoltage, ui->lblTemp,
        ui->lblPeriod, m_LocalPdValue, ui->lblDC, ui->lblName,
        ui->lblTimer
    };
    for (QWidget *value : values) {
        value->setFont(valueFont);
        value->setStyleSheet(valueStyle);
        if (QLabel *label = qobject_cast<QLabel *>(value))
            label->setAlignment(Qt::AlignCenter);
        if (QLineEdit *edit = qobject_cast<QLineEdit *>(value)) {
            edit->setMinimumHeight(36);
            edit->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        }
    }

    auto addField = [layout](int fieldRow, int column, QWidget *title, QWidget *value) {
        const int row = fieldRow * 2;
        layout->addWidget(title, row, column);
        layout->addWidget(value, row + 1, column);
    };

    addField(0, 0, ui->label_2, ui->lblFreq);
    addField(0, 1, ui->label_12, ui->lblIsppa);
    addField(0, 2, ui->label_5, ui->lblIspta);
    addField(0, 3, ui->label, ui->lblPower);
    addField(1, 0, ui->label_4, ui->label_6);
    addField(1, 1, ui->label_11, ui->lblDepth);
    addField(1, 2, ui->label_9, ui->lblVoltage);
    addField(1, 3, ui->label_3, ui->lblTemp);
    addField(2, 0, ui->label_15, ui->lblPeriod);
    addField(2, 1, m_LocalPdTitle, m_LocalPdValue);
    addField(2, 2, ui->label_14, ui->lblDC);
    addField(2, 3, ui->label_8, ui->lblName);
    addField(3, 0, ui->label_13, ui->lblTimer);

    layout->setContentsMargins(22, 16, 22, 12);
    layout->setHorizontalSpacing(20);
    layout->setVerticalSpacing(10);
    for (int column = 0; column < 4; ++column)
        layout->setColumnStretch(column, 1);
}

void MainWindowHIFU::UpdateLocalDerivedValues()
{
    if (!m_LocalPdValue)
        return;
    const double isppa = ui->lblIsppa->text().toDouble();
    const double priMs = ui->lblPeriod->text().toDouble();
    const double dutyPercent = ui->lblDC->text().toDouble();
    const double pdMs = priMs * dutyPercent / 100.0;
    const double rip = m_DataManager->m_CurrentProfile.isNull()
            ? 2.5
            : qMax(0.000001, m_DataManager->m_CurrentProfile->rip);
    ui->lblIspta->setText(QString::number(isppa * dutyPercent / 100.0, 'f', 2));
    ui->lblPower->setText(QString::number(isppa / rip, 'f', 1));
    m_LocalPdValue->setText(QString::number(pdMs, 'f', 1));
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

    const QString editStyle = QStringLiteral(
                "color: rgb(111,217,236); font-size: 26px; "
                "background: transparent; border: none;");
    const QString viewStyle = QStringLiteral(
                "color: rgb(255,255,255); font-size: 26px; "
                "background: transparent; border: none;");
    const QList<QLineEdit *> localEditableFields = {
        ui->lblDepth, ui->lblIsppa, ui->lblVoltage, ui->lblTimer,
        ui->lblDC, ui->lblPeriod, ui->lblName
    };
    for (QLineEdit *edit : localEditableFields) {
        edit->setStyleSheet(isEdit ? editStyle : viewStyle);
        SetUnderline(edit, isEdit);
    }
    UpdateLocalDerivedValues();
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
