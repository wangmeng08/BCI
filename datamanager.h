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
    bool IsNameUseLIFU(QString profileName, int excepetIndexID = 0);

    bool SaveInfoToCurrentProfile(QSharedPointer<Profile> profile);
    bool SaveInfoToCurrentProfileLIFU(QSharedPointer<ProfileLIFU> profile);

    bool SaveInfoToDefaultProfile(QSharedPointer<Profile> profile);
    bool SaveInfoToDefaultProfileLIFU(QSharedPointer<ProfileLIFU> profile);

    bool SaveInfoToNewProfile(QSharedPointer<Profile> profile);
    bool SaveInfoToNewProfileLIFU(QSharedPointer<ProfileLIFU> profile);

    void GetAllProfileInfo();
    void GetAllProfileInfoLIFU();

    void SetClinicalMode(ClinicalMode mode);

    void SetOptionInfo(bool isUsePowerLimit, bool isAdvanceRFMode, TriggerMode mode);

    ClinicalMode  GetClinicalMode ();
signals:
    void advanceRFModeChange();
    void powerLimitModeChange();
    void triggerModeChange();
public:
    QVector<QSharedPointer<Profile>> m_ProfileList;
    QVector<QSharedPointer<ProfileLIFU>> m_ProfileListLIFU;
    QSharedPointer<Profile> m_CurrentProfile;
    QSharedPointer<ProfileLIFU> m_CurrentProfileLIFU;
    bool m_IsUsePowerLimit = true;
    bool m_IsAdvanceRFMode = false;
    TriggerMode m_TriggerMode = TriggerMode::INTER;

private:
    ClinicalMode m_ClinicalMode = ClinicalMode::HIFU;
};

#endif // DATAMANAGER_H
