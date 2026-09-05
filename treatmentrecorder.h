#ifndef TREATMENTRECORDER_H
#define TREATMENTRECORDER_H

#include <QObject>
#include <QSharedPointer>

class Patient;
class Profile;

class TreatmentRecorder : public QObject
{
    Q_OBJECT
public:
    explicit TreatmentRecorder(QObject *parent = nullptr);
    static TreatmentRecorder *GetInstance();

    QString JsonPath() const;
    QString CsvPath() const;

    void RecordHifuTreatment(const Patient *patient,
                             const Profile *profile,
                             const QString &mode,
                             int actualTreatmentMs,
                             int pulsesDelivered = 1,
                             int pulseTrainDurationMs = 0,
                             int stimIntervalMs = 0,
                             int stimDurationMs = 0);

private:
    QString CsvEscape(const QString &value) const;
    QString AnimalName(const Patient *patient) const;
    void EnsureCsvHeader();

private:
    QString m_SessionId;
    QString m_JsonPath;
    QString m_CsvPath;
    int m_TreatmentCount = 0;
};

#endif // TREATMENTRECORDER_H
