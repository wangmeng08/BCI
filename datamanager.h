#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include "db.h"
#include "enuminfo.h"

class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = nullptr);
    static DataManager* GetInstance();

    bool IsNameUse(QString profileName, int excepetIndexID = 0);

    bool SaveInfoToCurrentProfile(QSharedPointer<Profile> profile);
    bool SaveInfoToDefaultProfile(QSharedPointer<Profile> profile);
    bool SaveInfoToNewProfile(QSharedPointer<Profile> profile);

    void SetOptionInfo(bool isUsePowerLimit, bool isAdvanceRFMode, TriggerMode mode);
signals:
    void advanceRFModeChange();
    void powerLimitModeChange();
    void triggerModeChange();
public:
    QVector<QSharedPointer<Profile>> m_ProfileList;
    QSharedPointer<Profile> m_CurrentProfile;
    bool m_IsUsePowerLimit = true;
    bool m_IsAdvanceRFMode = false;
    ClinicalMode m_ClinicalMode = ClinicalMode::HIFU;
    TriggerMode m_TriggerMode = TriggerMode::INTER;
};

#endif // DATAMANAGER_H
