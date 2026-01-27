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

bool DataManager::SaveInfoToNewProfile(QSharedPointer<Profile> profile)
{
    if(IsNameUse(profile->profileName, m_CurrentProfile->indexId))
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
DataManager::DataManager(QObject *parent)
    : QObject{parent}
{
    m_ProfileList = DB::GetInstance()->ProfileGetAllInfo();
    if(m_ProfileList.size()==0)
        return;
    m_CurrentProfile = m_ProfileList[0];
}
