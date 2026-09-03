#ifndef HIFUPULSETRAINADVANCE_H
#define HIFUPULSETRAINADVANCE_H

#include <QWidget>

class Profile;

namespace Ui {
class HifuPulseTrainAdvance;
}

class HifuPulseTrainAdvance : public QWidget
{
    Q_OBJECT
public:
    explicit HifuPulseTrainAdvance(QWidget *parent = nullptr);
    ~HifuPulseTrainAdvance();

    void LoadFromProfile(const Profile *profile);
    void SetEditMode(bool enabled);

    double isppa() const;
    double frequencyKhz() const;
    int priMs() const;
    int dutyCyclePercent() const;
    int pulseTrainDurationMs() const;
    double pulseDurationMs() const;
    double stimDurationS() const;
    double stimIntervalS() const;

private:
    void InitUiState();
    void UpdateDerivedValues();
    double lineValue(QObject *lineEdit, double fallback = 0.0) const;
    void setLineValue(QObject *lineEdit, double value, int precision = 1);

private:
    Ui::HifuPulseTrainAdvance *ui;
};

#endif // HIFUPULSETRAINADVANCE_H
