#include "treatmentrecorder.h"

#include "constvalue.h"
#include "db.h"
#include "eventmanager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

Q_GLOBAL_STATIC(TreatmentRecorder, treatmentRecorder);

TreatmentRecorder *TreatmentRecorder::GetInstance()
{
    return treatmentRecorder();
}

TreatmentRecorder::TreatmentRecorder(QObject *parent)
    : QObject(parent)
{
    m_SessionId = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString reportDir =
            QCoreApplication::applicationDirPath() + QStringLiteral("/reports");
    QDir().mkpath(reportDir);
    m_JsonPath = reportDir + QStringLiteral("/treatment_") + m_SessionId
            + QStringLiteral(".jsonl");
    m_CsvPath = reportDir + QStringLiteral("/treatment_") + m_SessionId
            + QStringLiteral(".csv");
    EnsureCsvHeader();
}

QString TreatmentRecorder::JsonPath() const
{
    return m_JsonPath;
}

QString TreatmentRecorder::CsvPath() const
{
    return m_CsvPath;
}

void TreatmentRecorder::RecordHifuTreatment(const Patient *patient,
                                            const Profile *profile,
                                            const QString &mode,
                                            int actualTreatmentMs,
                                            int pulsesDelivered,
                                            int pulseTrainDurationMs,
                                            int stimIntervalMs,
                                            int stimDurationMs)
{
    if (!profile)
        return;

    ++m_TreatmentCount;
    const QString timestamp =
            QDateTime::currentDateTime().toString(Qt::ISODate);
    const double pdMs = qMax(1, profile->period)
            * qBound(0, profile->dc, 100) / 100.0;

    QJsonObject patientObject;
    patientObject.insert(QStringLiteral("id"), patient ? patient->index : 0);
    patientObject.insert(QStringLiteral("name"), patient ? patient->name : QString());
    patientObject.insert(QStringLiteral("animal_type"), AnimalName(patient));
    patientObject.insert(QStringLiteral("illness"), patient ? patient->illness : QString());
    patientObject.insert(QStringLiteral("age"), patient ? patient->age : 0);
    patientObject.insert(QStringLiteral("weight"), patient ? patient->weight : 0.0);

    QJsonObject parameters;
    parameters.insert(QStringLiteral("profile_name"), profile->profileName);
    parameters.insert(QStringLiteral("frequency_khz"), profile->freq);
    parameters.insert(QStringLiteral("isppa_w_cm2"), profile->isppa);
    parameters.insert(QStringLiteral("ispta_w_cm2"), profile->GetIspta());
    parameters.insert(QStringLiteral("power_w"), profile->GetPower());
    parameters.insert(QStringLiteral("pri_ms"), profile->period);
    parameters.insert(QStringLiteral("pd_ms"), pdMs);
    parameters.insert(QStringLiteral("duty_cycle_percent"), profile->dc);
    parameters.insert(QStringLiteral("actual_treatment_ms"), actualTreatmentMs);
    parameters.insert(QStringLiteral("pulses_delivered"), pulsesDelivered);
    parameters.insert(QStringLiteral("pulse_train_duration_ms"), pulseTrainDurationMs);
    parameters.insert(QStringLiteral("stim_interval_ms"), stimIntervalMs);
    parameters.insert(QStringLiteral("stim_duration_ms"), stimDurationMs);
    parameters.insert(QStringLiteral("depth_mm"), profile->depth);
    parameters.insert(QStringLiteral("voltage_v"), profile->voltage);

    QJsonObject record;
    record.insert(QStringLiteral("session_id"), m_SessionId);
    record.insert(QStringLiteral("treatment_index"), m_TreatmentCount);
    record.insert(QStringLiteral("timestamp"), timestamp);
    record.insert(QStringLiteral("clinical_mode"), QStringLiteral("HIFU"));
    record.insert(QStringLiteral("mode"), mode);
    record.insert(QStringLiteral("patient"), patientObject);
    record.insert(QStringLiteral("parameters"), parameters);

    QFile jsonFile(m_JsonPath);
    if (jsonFile.open(QIODevice::Append | QIODevice::Text)) {
        jsonFile.write(QJsonDocument(record).toJson(QJsonDocument::Compact));
        jsonFile.write("\n");
    }

    QFile csvFile(m_CsvPath);
    if (csvFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&csvFile);
        out << m_TreatmentCount << ','
            << CsvEscape(timestamp) << ','
            << CsvEscape(QStringLiteral("HIFU")) << ','
            << CsvEscape(mode) << ','
            << actualTreatmentMs << ','
            << pulsesDelivered << ','
            << (patient ? patient->index : 0) << ','
            << CsvEscape(patient ? patient->name : QString()) << ','
            << CsvEscape(AnimalName(patient)) << ','
            << CsvEscape(profile->profileName) << ','
            << profile->freq << ','
            << profile->isppa << ','
            << profile->GetIspta() << ','
            << profile->GetPower() << ','
            << profile->period << ','
            << pdMs << ','
            << profile->dc << ','
            << pulseTrainDurationMs << ','
            << stimIntervalMs << ','
            << stimDurationMs << ','
            << profile->depth << ','
            << profile->voltage << '\n';
    }

    emit EventManager::GetInstance()->writeLog(
                LogType::INFO,
                QStringLiteral("Treatment recorded #%1: %2")
                .arg(m_TreatmentCount)
                .arg(m_CsvPath));
}

QString TreatmentRecorder::CsvEscape(const QString &value) const
{
    QString escaped = value;
    escaped.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    if (escaped.contains(',') || escaped.contains('\n') || escaped.contains('\r'))
        return QStringLiteral("\"%1\"").arg(escaped);
    return escaped;
}

QString TreatmentRecorder::AnimalName(const Patient *patient) const
{
    if (!patient)
        return QString();
    const QStringList animals = ConstValue::GetInstance()->AnimalList;
    if (patient->animalType < 0 || patient->animalType >= animals.size())
        return QString::number(patient->animalType);
    return animals[patient->animalType];
}

void TreatmentRecorder::EnsureCsvHeader()
{
    QFile csvFile(m_CsvPath);
    if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&csvFile);
    out << "TreatmentIndex,Timestamp,ClinicalMode,Mode,ActualTreatmentMs,PulsesDelivered,"
           "PatientId,PatientName,AnimalType,ProfileName,FrequencyKHz,IsppaWcm2,"
           "IsptaWcm2,PowerW,PRIMs,PDMs,DutyCyclePercent,"
           "PulseTrainDurationMs,StimIntervalMs,StimDurationMs,DepthMm,VoltageV\n";
}
