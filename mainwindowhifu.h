#ifndef MAINWINDOWHIFU_H
#define MAINWINDOWHIFU_H

#include <QMainWindow>
#include <QLineEdit>
#include "basewindow.h"
#include "datamanager.h"
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowHIFU; }
QT_END_NAMESPACE

class HifuPulseTrainAdvance;

class MainWindowHIFU : public BaseWindow
{
    Q_OBJECT

public:
    MainWindowHIFU(QWidget *parent = nullptr);
    ~MainWindowHIFU();

    void InitProfileData() override;
protected:
    QLabel *GetConnectLabel() override;
    QLabel *GetEmitLabel() override;
    QLabel *GetStateIcon() override;
    void SendInitCommand() override;
    void SetTimerInfo() override;
    void OnSonicStarted(uint8_t addr) override;
    void OnSonicStopped() override;

private:
    void InitData();
    void InitEvent();

    void OnClickAdvance();
    void OnClickCancel();
    void OnClickEdit();
    void OnClickLoad();
    void OnClickLocal();
    void OnClickOff();
    void OnClickOn();
    void OnClickOption();
    void OnClickSave();

    void OnCurrentProfileChange(QSharedPointer<Profile> prev, QSharedPointer<Profile> curr);

    void OnModeAdvanceRFChange();
    void OnModePowerLimitChange();
    void OnModeTriggerChange();

    void SendSetAfterSave(bool isInit=false);

    void ApplyPulseTrainAdvanceToProfile(Profile *profile) const;
    void StartPulseTrain();
    void StopPulseTrain(bool sendStopCommand);
    void RecordLocalTreatmentIfNeeded();
    void RecordPulseTrainTreatmentIfNeeded(int actualTreatmentMs);
    void OnPulseTrainStartPause();
    void PausePulseTrain();
    void ResumePulseTrain();
    void PulseTrainTick();
    void PulseTrainDisplayTick();
    int PulseTrainCount() const;
    void SetAdvanceBtnState();
    void SetEditMode(bool isEdit);

    void UpdateBtnState() override;

private:
    QVector<QLineEdit *> m_VectorListDelay;
    QVector<QLineEdit *> m_VectorListFreq;
    QVector<QLineEdit *> m_VectorListHCD;
    HifuPulseTrainAdvance *m_PulseTrainAdvance = nullptr;
    QTimer *m_PulseTrainTimer = nullptr;
    QTimer *m_PulseTrainDisplayTimer = nullptr;
    QSharedPointer<Profile> m_PulseTrainProfile;
    int m_PulseTrainSentCount = 0;
    int m_PulseTrainTotalCount = 0;
    int m_PulseTrainIntervalMs = 0;
    int m_PulseTrainPulseTimerMs = 0;
    int m_PulseTrainTotalDurationMs = 0;
    int m_PulseTrainRemainingMs = 0;
    bool m_PulseTrainActive = false;
    bool m_PulseTrainPaused = false;
    bool m_PulseTrainRecorded = false;
    bool m_PulseTrainStarted = false;
    bool m_LocalTreatmentStarted = false;
    bool m_LocalTreatmentRecorded = false;
    bool m_IsInAdvance = false;
    bool m_IsInEdit = false;
    Ui::MainWindowHIFU *ui;
};
#endif // MAINWINDOWHIFU_H
