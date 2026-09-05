#include "hifupulsetrainadvance.h"
#include "ui_hifupulsetrainadvance.h"
#include "db.h"

#include <QLineEdit>
#include <QPushButton>

HifuPulseTrainAdvance::HifuPulseTrainAdvance(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HifuPulseTrainAdvance)
{
    ui->setupUi(this);
    InitUiState();
}

HifuPulseTrainAdvance::~HifuPulseTrainAdvance()
{
    delete ui;
}

void HifuPulseTrainAdvance::InitUiState()
{
    const QList<QLineEdit *> edits = {
        ui->editIsppa,
        ui->editIspta,
        ui->editFrequency,
        ui->editPower,
        ui->editPri,
        ui->editDutyCycle,
        ui->editPd,
        ui->editStimDuration,
        ui->editStimInterval,
        ui->editPulseTrainDuration
    };
    for (QLineEdit *edit : edits)
        edit->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    ui->editIspta->setReadOnly(true);
    ui->editPower->setReadOnly(true);
    ui->editFrequency->setReadOnly(true);
    ui->editPd->setReadOnly(true);
    ResetProgress();
    SetPulseTrainRunning(false);

    connect(ui->editPri, &QLineEdit::editingFinished,
            this, &HifuPulseTrainAdvance::UpdateDerivedValues);
    connect(ui->editDutyCycle, &QLineEdit::editingFinished,
            this, &HifuPulseTrainAdvance::UpdateDerivedValues);
    connect(ui->editIsppa, &QLineEdit::editingFinished,
            this, &HifuPulseTrainAdvance::UpdateDerivedValues);
    connect(ui->btnStartPause, &QPushButton::clicked,
            this, &HifuPulseTrainAdvance::startPauseRequested);
    connect(ui->btnStop, &QPushButton::clicked,
            this, &HifuPulseTrainAdvance::stopRequested);
}

void HifuPulseTrainAdvance::LoadFromProfile(const Profile *profile)
{
    if (!profile)
        return;

    m_Rip = profile->rip <= 0.0 ? 1.0 : profile->rip;
    setLineValue(ui->editIsppa, profile->isppa, 1);
    setLineValue(ui->editFrequency, profile->freq, 1);
    setLineValue(ui->editPower, profile->GetPower(), 1);
    setLineValue(ui->editPri, profile->period, 1);
    setLineValue(ui->editDutyCycle, profile->dc, 1);
    setLineValue(ui->editPulseTrainDuration, profile->timer, 1);
    setLineValue(ui->editStimDuration, 150.0, 1);
    setLineValue(ui->editStimInterval, 0.7, 1);
    UpdateDerivedValues();
}

void HifuPulseTrainAdvance::SetEditMode(bool enabled)
{
    Q_UNUSED(enabled);
    ui->editIsppa->setEnabled(true);
    ui->editPri->setEnabled(true);
    ui->editDutyCycle->setEnabled(true);
    ui->editStimDuration->setEnabled(true);
    ui->editStimInterval->setEnabled(true);
    ui->editPulseTrainDuration->setEnabled(true);
}

void HifuPulseTrainAdvance::ResetProgress()
{
    UpdateProgress(0, 0, 0);
}

void HifuPulseTrainAdvance::UpdateProgress(int elapsedMs, int remainingMs, int totalMs)
{
    const int boundedTotalMs = qMax(0, totalMs);
    const int boundedElapsedMs = qBound(0, elapsedMs, boundedTotalMs);
    const int boundedRemainingMs = qMax(0, remainingMs);
    const int progressMax = boundedTotalMs > 0 ? boundedTotalMs : 100;
    const int progressValue = boundedTotalMs > 0 ? boundedElapsedMs : 0;

    ui->progressTreatment->setRange(0, progressMax);
    ui->progressTreatment->setValue(progressValue);
    ui->lblElapsedValue->setText(QStringLiteral("%1 s")
                                 .arg(boundedElapsedMs / 1000.0, 0, 'f', 1));
    ui->lblRemainingValue->setText(QStringLiteral("%1 s")
                                   .arg(boundedRemainingMs / 1000.0, 0, 'f', 1));
}

void HifuPulseTrainAdvance::SetPulseTrainRunning(bool running, bool paused)
{
    ui->btnStartPause->setText(running && !paused ? QStringLiteral("Pause")
                                                  : QStringLiteral("Start"));
    ui->btnStop->setEnabled(running || paused);
}

double HifuPulseTrainAdvance::isppa() const
{
    return lineValue(ui->editIsppa);
}

double HifuPulseTrainAdvance::frequencyKhz() const
{
    return lineValue(ui->editFrequency, 500.0);
}

int HifuPulseTrainAdvance::priMs() const
{
    return qMax(1, qRound(lineValue(ui->editPri, 10.0)));
}

int HifuPulseTrainAdvance::dutyCyclePercent() const
{
    return qBound(0, qRound(lineValue(ui->editDutyCycle, 50.0)), 100);
}

int HifuPulseTrainAdvance::pulseTrainDurationMs() const
{
    return qMax(1, qRound(lineValue(ui->editPulseTrainDuration, 30.0)));
}

double HifuPulseTrainAdvance::pulseDurationMs() const
{
    return qMax(0.0, lineValue(ui->editPd, 5.0));
}

double HifuPulseTrainAdvance::stimDurationS() const
{
    return qMax(0.0, lineValue(ui->editStimDuration, 150.0));
}

double HifuPulseTrainAdvance::stimIntervalS() const
{
    return qMax(0.0, lineValue(ui->editStimInterval, 0.7));
}

void HifuPulseTrainAdvance::UpdateDerivedValues()
{
    const double pri = qMax(1.0, lineValue(ui->editPri, 10.0));
    double duty = qBound(0.0, lineValue(ui->editDutyCycle, 50.0), 100.0);
    const double pulseDuration = pri * duty / 100.0;
    const double isppaValue = lineValue(ui->editIsppa, 0.0);

    setLineValue(ui->editPd, pulseDuration, 1);
    setLineValue(ui->editIspta, isppaValue * duty / 100.0, 2);
    setLineValue(ui->editPower, m_Rip <= 0.0 ? isppaValue : isppaValue / m_Rip, 1);
}

double HifuPulseTrainAdvance::lineValue(QObject *lineEdit, double fallback) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(lineEdit);
    if (!edit)
        return fallback;

    bool ok = false;
    const double value = edit->text().toDouble(&ok);
    return ok ? value : fallback;
}

void HifuPulseTrainAdvance::setLineValue(QObject *lineEdit, double value, int precision)
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(lineEdit);
    if (!edit)
        return;
    edit->setText(QString::number(value, 'f', precision));
}
