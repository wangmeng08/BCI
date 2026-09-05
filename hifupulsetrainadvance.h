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
    void ResetProgress();
    void UpdateProgress(int elapsedMs, int remainingMs, int totalMs);
    void SetPulseTrainRunning(bool running, bool paused = false);

    double isppa() const;
    double frequencyKhz() const;
    int priMs() const;
    int dutyCyclePercent() const;
    int pulseTrainDurationMs() const;
    double pulseDurationMs() const;
    double stimDurationS() const;
    double stimIntervalS() const;

signals:
    void startPauseRequested();
    void stopRequested();

private:
    void InitUiState();
    void UpdateDerivedValues();
    double lineValue(QObject *lineEdit, double fallback = 0.0) const;
    void setLineValue(QObject *lineEdit, double value, int precision = 1);

private:
    Ui::HifuPulseTrainAdvance *ui;
    double m_Rip = 1.0;
};

#endif // HIFUPULSETRAINADVANCE_H
