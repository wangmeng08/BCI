#include "hifupulsetrainadvance.h"
#include "ui_hifupulsetrainadvance.h"
#include "db.h"

#include <QLineEdit>

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
        ui->editPulseDuration,
        ui->editStimDuration,
        ui->editStimInterval,
        ui->editPulseTrainDuration
    };
    for (QLineEdit *edit : edits)
        edit->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    ui->editIspta->setReadOnly(true);

    connect(ui->editPri, &QLineEdit::editingFinished,
            this, &HifuPulseTrainAdvance::UpdateDerivedValues);
    connect(ui->editDutyCycle, &QLineEdit::editingFinished,
            this, &HifuPulseTrainAdvance::UpdateDerivedValues);
    connect(ui->editPulseDuration, &QLineEdit::editingFinished,
            this, &HifuPulseTrainAdvance::UpdateDerivedValues);
    connect(ui->editIsppa, &QLineEdit::editingFinished,
            this, &HifuPulseTrainAdvance::UpdateDerivedValues);
}

void HifuPulseTrainAdvance::LoadFromProfile(const Profile *profile)
{
    if (!profile)
        return;

    setLineValue(ui->editIsppa, profile->isppa, 1);
    setLineValue(ui->editFrequency, profile->freq, 1);
    setLineValue(ui->editPower, profile->GetPower(), 1);
    setLineValue(ui->editPri, profile->period, 1);
    setLineValue(ui->editDutyCycle, profile->dc, 1);
    setLineValue(ui->editPulseDuration, profile->burstLen, 1);
    setLineValue(ui->editPulseTrainDuration, profile->timer, 1);
    setLineValue(ui->editStimDuration, 150.0, 1);
    setLineValue(ui->editStimInterval, 0.7, 1);
    UpdateDerivedValues();
}

void HifuPulseTrainAdvance::SetEditMode(bool enabled)
{
    Q_UNUSED(enabled);
    ui->editIsppa->setEnabled(true);
    ui->editPower->setEnabled(true);
    ui->editPri->setEnabled(true);
    ui->editDutyCycle->setEnabled(true);
    ui->editPulseDuration->setEnabled(true);
    ui->editStimDuration->setEnabled(true);
    ui->editStimInterval->setEnabled(true);
    ui->editPulseTrainDuration->setEnabled(true);
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
    return qMax(0.0, lineValue(ui->editPulseDuration, 5.0));
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
    double pulseDuration = pri * duty / 100.0;
    const double isppaValue = lineValue(ui->editIsppa, 0.0);

    if (sender() == ui->editPri || sender() == ui->editDutyCycle) {
        setLineValue(ui->editPulseDuration, pulseDuration, 1);
    }
    else {
        pulseDuration = qBound(0.0, lineValue(ui->editPulseDuration, 5.0), pri);
        duty = pulseDuration * 100.0 / pri;
        setLineValue(ui->editDutyCycle, duty, 1);
    }
    setLineValue(ui->editIspta, isppaValue * duty / 100.0, 2);
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
