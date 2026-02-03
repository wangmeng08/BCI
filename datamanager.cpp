#include "datamanager.h"
#include "messageinfo.h"
Q_GLOBAL_STATIC(DataManager, dataManager);

DataManager* DataManager::GetInstance()
{
    return dataManager();
}

bool DataManager::CreatePatient(QSharedPointer<Patient> patient)
{
    if(IsNameUsePatient(patient->name))
    {
        MessageInfo::ShowInformation(tr("Save failed, the patient name is already in use"));
        return false;
    }
    bool res = DB::GetInstance()->PatientCreate(patient);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Create a new patient failed"));
    }
    else{
        m_PatientList.append(patient);
        MessageInfo::ShowInformation(tr("Save a new patient success"));
    }
    return res;
}

bool DataManager::CreatePatient(QString name, QString illness, QString age, QString weight, int kind)
{

    if(IsNameUsePatient(name))
    {
        MessageInfo::ShowInformation(tr("Save failed, the patient name is already in use"));
        return false;
    }
    QSharedPointer<Patient> patient = QSharedPointer<Patient>::create();
    patient->age = age.toInt();
    patient->animalType = kind;
    patient->illness = illness;
    patient->name = name;
    patient->weight = weight.toDouble();
    return CreatePatient(patient);
}

bool DataManager::CreateReport(QSharedPointer<Report> report)
{
    if(m_CurrentPatient.isNull())
    {
        MessageInfo::ShowInformation(tr("Current patient is null"));
    }
    report->patientIndex = m_CurrentPatient->index;
    bool res = DB::GetInstance()->ReportCreate(report);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Create a new report failed"));
    }
    else{
        if(m_Report.contains(m_CurrentPatient->index))
        {
            QVector<QSharedPointer<Report>> vector;
            m_Report[report->patientIndex] = vector;
        }
        m_Report[report->patientIndex].append(report);
        MessageInfo::ShowInformation(tr("Save a new report success"));
    }
    return res;
}

bool DataManager::DeletePatient(QSharedPointer<Patient> patient)
{
    bool res = DB::GetInstance()->PatientDelete(patient->index);
    if(res)
    {
        m_PatientList.removeOne(patient);
    }
    return res;
}

bool DataManager::IsNameUse(QString profileName, int excepetIndexID)
{
    for(const auto &profile : qAsConst(m_ProfileList))
    {
        if(profile->profileName == profileName && profile->indexId != excepetIndexID)
            return true;
    }
    return false;
}

bool DataManager::IsNameUseLIFU(QString profileName, int excepetIndexID)
{
    for(const auto &profile : qAsConst(m_ProfileListLIFU))
    {
        if(profile->profileName == profileName && profile->indexId != excepetIndexID)
            return true;
    }
    return false;
}

bool DataManager::IsNameUsePatient(QString name)
{
    for(const auto &patient : qAsConst(m_PatientList))
    {
        if(patient->name == name)
            return true;
    }
    return false;
}

bool DataManager::SaveInfoToCurrentProfile(QSharedPointer<Profile> profile)
{
    if(m_CurrentProfile->indexId == 1 && profile->profileName != m_CurrentProfile->profileName)
    {
        MessageInfo::ShowInformation(tr("Save failed, the default profile's name cannot be modified"));
        return false;
    }
    if(IsNameUse(profile->profileName, m_CurrentProfile->indexId))
    {
        MessageInfo::ShowInformation(tr("Save failed, the profile name is already in use"));
        return false;
    }
    bool res = DB::GetInstance()->ProfileModifyInfo(profile, m_CurrentProfile);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Save profile failed"));
    }
    else{
        MessageInfo::ShowInformation(tr("Save profile info success"));
    }
    return res;
}

bool DataManager::SaveInfoToCurrentProfileLIFU(QSharedPointer<ProfileLIFU> profile)
{
    if(m_CurrentProfileLIFU->indexId == 1 && profile->profileName != m_CurrentProfileLIFU->profileName)
    {
        MessageInfo::ShowInformation(tr("Save failed, the default profile's name cannot be modified"));
        return false;
    }
    if(IsNameUseLIFU(profile->profileName, m_CurrentProfileLIFU->indexId))
    {
        MessageInfo::ShowInformation(tr("Save failed, the profile name is already in use"));
        return false;
    }
    bool res = DB::GetInstance()->ProfileModifyInfoLIFU(profile, m_CurrentProfileLIFU);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Save profile failed"));
    }
    else{
        MessageInfo::ShowInformation(tr("Save profile info success"));
    }
    return res;
}

bool DataManager::SaveInfoToDefaultProfile(QSharedPointer<Profile> profile)
{
    QSharedPointer<Profile> defaultProfile = m_ProfileList[0];
    profile->profileName = defaultProfile->profileName;
    bool res = DB::GetInstance()->ProfileModifyInfo(profile, defaultProfile);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Save info to default profile failed"));
    }
    else{
        m_CurrentProfile = defaultProfile;
        MessageInfo::ShowInformation(tr("Save info to default profile success"));
    }
    return res;
}

bool DataManager::SaveInfoToDefaultProfileLIFU(QSharedPointer<ProfileLIFU> profile)
{
    QSharedPointer<ProfileLIFU> defaultProfile = m_ProfileListLIFU[0];
    profile->profileName = defaultProfile->profileName;
    bool res = DB::GetInstance()->ProfileModifyInfoLIFU(profile, defaultProfile);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Save info to default profile failed"));
    }
    else{
        m_CurrentProfileLIFU = defaultProfile;
        MessageInfo::ShowInformation(tr("Save info to default profile success"));
    }
    return res;
}

bool DataManager::SaveInfoToNewProfile(QSharedPointer<Profile> profile)
{
    if(IsNameUse(profile->profileName))
    {
        MessageInfo::ShowInformation(tr("Save failed, the profile name is already in use"));
        return false;
    }
    bool res = DB::GetInstance()->ProfileCreate(profile);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Save a new profile failed"));
    }
    else{
        m_ProfileList.append(profile);
        m_CurrentProfile = profile;
        MessageInfo::ShowInformation(tr("Save a new profile success"));
    }
    return res;
}

bool DataManager::SaveInfoToNewProfileLIFU(QSharedPointer<ProfileLIFU> profile)
{
    if(IsNameUseLIFU(profile->profileName))
    {
        MessageInfo::ShowInformation(tr("Save failed, the profile name is already in use"));
        return false;
    }
    bool res = DB::GetInstance()->ProfileCreateLIFU(profile);
    if(!res)
    {
        MessageInfo::ShowInformation(tr("Save a new profile failed"));
    }
    else{
        m_ProfileListLIFU.append(profile);
        m_CurrentProfileLIFU = profile;
        MessageInfo::ShowInformation(tr("Save a new profile success"));
    }
    return res;
}

void DataManager::GetAllPatient()
{
    DB::GetInstance()->PatientGetAllInfo(m_PatientList);
}

void DataManager::GetAllProfileInfo()
{
    DB::GetInstance()->ProfileGetAllInfo(m_ProfileList);
    if(m_ProfileList.size()==0)
        return;
    m_CurrentProfile = m_ProfileList[0];
}

void DataManager::GetAllProfileInfoLIFU()
{
    DB::GetInstance()->ProfileGetAllInfoLIFU(m_ProfileListLIFU);
    if(m_ProfileListLIFU.size()==0)
        return;
    m_CurrentProfileLIFU = m_ProfileListLIFU[0];
}

void DataManager::GetAllReport()
{
    DB::GetInstance()->ReportGetAllInfo(m_Report);
}

void DataManager::SetClinicalMode(ClinicalMode mode)
{
    m_ClinicalMode = mode;
    if(mode == ClinicalMode::HIFU)
    {
        GetAllProfileInfo();
    }
    else if(mode == ClinicalMode::LIFU)
    {
        GetAllProfileInfoLIFU();
    }
}

void DataManager::SetOptionInfo(bool isUsePowerLimit, bool isAdvanceRFMode, TriggerMode mode)
{
    if(m_IsUsePowerLimit != isUsePowerLimit)
    {
        m_IsUsePowerLimit = isUsePowerLimit;
        emit powerLimitModeChange();
    }
    if(m_IsAdvanceRFMode != isAdvanceRFMode)
    {
        m_IsAdvanceRFMode = isAdvanceRFMode;
        emit advanceRFModeChange();
    }
    if(m_TriggerMode != mode)
    {
        m_TriggerMode = mode;
        emit triggerModeChange();
    }
}

ClinicalMode DataManager::GetClinicalMode()
{
    return m_ClinicalMode;
}
DataManager::DataManager(QObject *parent)
    : QObject{parent}
{
    GetAllProfileInfo();
    GetAllProfileInfoLIFU();
    GetAllPatient();
    GetAllReport();
}
