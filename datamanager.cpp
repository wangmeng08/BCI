#include "datamanager.h"
#include "messageinfo.h"
Q_GLOBAL_STATIC(DataManager, dataManager);

DataManager* DataManager::GetInstance()
{
    return dataManager();
}

bool DataManager::IsNameUse(QString profileName, int excepetIndexID)
{
    for(const auto &profile : m_ProfileList)
    {
        if(profile->profileName == profileName && profile->indexId != excepetIndexID)
            return true;
    }
    return false;
}

bool DataManager::IsNameUseLIFU(QString profileName, int excepetIndexID)
{
    for(const auto &profile : m_ProfileListLIFU)
    {
        if(profile->profileName == profileName && profile->indexId != excepetIndexID)
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

void DataManager::GetAllProfileInfo()
{
    m_ProfileList = DB::GetInstance()->ProfileGetAllInfo();
    if(m_ProfileList.size()==0)
        return;
    m_CurrentProfile = m_ProfileList[0];
}

void DataManager::GetAllProfileInfoLIFU()
{
    m_ProfileListLIFU = DB::GetInstance()->ProfileGetAllInfoLIFU();
    if(m_ProfileListLIFU.size()==0)
        return;
    m_CurrentProfileLIFU = m_ProfileListLIFU[0];
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

}
