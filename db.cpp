#include "db.h"
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>
#include <iostream>
#include "eventmanager.h"

Q_GLOBAL_STATIC(DB, dbInstance)
DB::DB(QObject *parent)
    : QObject{parent}
{
    InitDataBase();
    CreateConnection();
    CreateTableProfile();
    CreateTableLIFUProfile();
    CreateTableLIFUProfileValue();
}

DB *DB::GetInstance()
{
    return dbInstance();
}

bool DB::CreateConnection()
{
    if (db.isOpen())
      return true;
    if (!db.open())
    {
      std::cout << "Error opening database:" << db.lastError().text().toStdString() << std::endl;
      return false;
    }
    return true;
}

void DB::CreateTableLIFUProfile()
{
    QSqlQuery query;
    QString sql = "CREATE TABLE IF NOT EXISTS lifu_profiles ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "profile_name TEXT NOT NULL UNIQUE, "
                    "dutyc REAL NOT NULL, "
                    "period INTEGER NOT NULL, "
                    "timer INTEGER NOT NULL, "
                    "temp REAL NOT NULL, "
                    "voltage REAL NOT NULL)";
    bool success = query.exec(sql);
    if (!success)
    {
        std::cout << "CreateTableLIFUProfile error1: " << query.lastError().text().toStdString();
        QString info = QString("create table lifu_profiles error: %1").arg(query.lastError().text());
        WriteLog(info);
    }
}

void DB::CreateTableLIFUProfileValue()
{
    QSqlQuery query;
    QString sql = "CREATE TABLE IF NOT EXISTS lifu_profile_value ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "profile_id INTEGER, "
                    "idx INTEGER, "
                    "value INTEGER, "
                    "FOREIGN KEY(profile_id) REFERENCES profile(id))";
    bool success = query.exec(sql);
    if (!success)
    {
        std::cout << "CreateTableLIFUProfileValue error1: " << query.lastError().text().toStdString();
        QString info = QString("create table lifu_profiles error: %1").arg(query.lastError().text());
        WriteLog(info);
    }
}

void DB::CreateTableProfile()
{
    QSqlQuery query;
    QString sql = "CREATE TABLE IF NOT EXISTS profiles ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "profile_name TEXT NOT NULL UNIQUE, "
                    "isppa REAL NOT NULL default 0, "
                    "rip REAL NOT NULL, "
                    "period INTEGER NOT NULL, "
                    "dc INTEGER NOT NULL, "
                    "timer INTEGER NOT NULL, "
                    "depth REAL NOT NULL, "
                    "freq REAL NOT NULL, "
                    "temp REAL NOT NULL, "
                    "voltage REAL NOT NULL, "
                    "burstLen REAL NOT NULL, "
                    "hcd1 REAL NOT NULL, "
                    "freq1 REAL NOT NULL, "
                    "delay1 REAL NOT NULL, "
                    "hcd2 REAL NOT NULL, "
                    "freq2 REAL NOT NULL, "
                    "delay2 REAL NOT NULL, "
                    "hcd3 REAL NOT NULL, "
                    "freq3 REAL NOT NULL, "
                    "delay3 REAL NOT NULL, "
                    "hcd4 REAL NOT NULL, "
                    "freq4 REAL NOT NULL, "
                    "delay4 REAL NOT NULL)";
    bool success = query.exec(sql);
    if (!success)
    {
        std::cout << "createTreatHistroyTable error1: " << query.lastError().text().toStdString();
        QString info = QString("create table profile error: %1").arg(query.lastError().text());
        WriteLog(info);
    }
}

void DB::InitDataBase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");

    QString storageDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QString dirpath = QString("%1/bci").arg(storageDir);
    QString filePath = QString("%1/profile.db").arg(dirpath);
    QDir dir(dirpath);
    if (!dir.exists())
      dir.mkdir(dirpath);
    db.setDatabaseName(filePath);
}

void DB::WriteLog(QString info)
{
    emit EventManager::GetInstance()->writeLog(LogType::DB, info);
}

bool DB::ProfileCreate(QSharedPointer<Profile> profile, bool isDefault)
{
    CreateConnection();
    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO profiles
        (profile_name, isppa, rip, period, dc, timer, depth, freq, temp, voltage, burstLen, hcd1, freq1, delay1, hcd2, freq2, delay2, hcd3,
             freq3, delay3, hcd4, freq4, delay4)
        VALUES
        (:profile_name, :isppa, :rip, :period, :dc, :timer, :depth, :freq, :temp, :voltage, :burstLen, :hcd1, :freq1, :delay1, :hcd2, :freq2,
            :delay2, :hcd3, :freq3, :delay3, :hcd4, :freq4, :delay4)
    )");

    query.bindValue(":profile_name", profile->profileName);
    query.bindValue(":isppa",      profile->isppa);
    query.bindValue(":rip",        profile->rip);
    query.bindValue(":period",     profile->period);
    query.bindValue(":dc",         profile->dc);
    query.bindValue(":timer",      profile->timer);
    query.bindValue(":depth",      profile->depth);
    query.bindValue(":freq",       profile->freq);
    query.bindValue(":temp",       profile->temp);
    query.bindValue(":voltage",    profile->voltage);
    query.bindValue(":burstLen",   profile->burstLen);
    query.bindValue(":hcd1",       profile->infoList[0]->hcd);
    query.bindValue(":freq1",      profile->infoList[0]->freq);
    query.bindValue(":delay1",     profile->infoList[0]->delay);
    query.bindValue(":hcd2",       profile->infoList[1]->hcd);
    query.bindValue(":freq2",      profile->infoList[1]->freq);
    query.bindValue(":delay2",     profile->infoList[1]->delay);
    query.bindValue(":hcd3",       profile->infoList[2]->hcd);
    query.bindValue(":freq3",      profile->infoList[2]->freq);
    query.bindValue(":delay3",     profile->infoList[2]->delay);
    query.bindValue(":hcd4",       profile->infoList[3]->hcd);
    query.bindValue(":freq4",      profile->infoList[3]->freq);
    query.bindValue(":delay4",     profile->infoList[3]->delay);

    bool success = query.exec();
    QString logInfo = QString("INSERT into profiles set %1").arg(profile->GetInfo());
    WriteLog(logInfo);
    if (!success)
    {
        QString info =  QString("profileCreateNew error2: %1").arg(query.lastError().text());
        WriteLog(info);
        if(isDefault)
        {

        }
    }
    else
    {
        profile->indexId = query.lastInsertId().toInt();
    }
    db.close();
    return success;
}

bool DB::ProfileCreateLIFU(QSharedPointer<ProfileLIFU> profile, bool isDefault)
{
    CreateConnection();
    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO lifu_profiles
        (profile_name, dutyc, period, timer, temp, voltage)
        VALUES
        (:profile_name, :dutyc, :period, :timer, :temp, :voltage)
    )");

    query.bindValue(":profile_name", profile->profileName);
    query.bindValue(":dutyc",      profile->dutyc);
    query.bindValue(":period",     profile->period);
    query.bindValue(":timer",      profile->timer);
    query.bindValue(":temp",       profile->temp);
    query.bindValue(":voltage",    profile->voltage);

    bool success = query.exec();
    QString logInfo = QString("INSERT into profiles_lifu set %1").arg(profile->GetInfo());
    WriteLog(logInfo);
    if (!success)
    {
        QString info =  QString("ProfileCreateLIFU error2: %1").arg(query.lastError().text());
        WriteLog(info);
        if(isDefault)
        {

        }
    }
    else
    {
        profile->indexId = query.lastInsertId().toInt();
        QSqlQuery q;
        q.prepare("INSERT INTO lifu_profile_value(profile_id, idx, value) VALUES(?, ?, ?)");

        for (int i = 0; i < ProfileLIFU::ValueCount; ++i)
        {
            q.addBindValue(profile->indexId);
            q.addBindValue(i);
            q.addBindValue(profile->values[i]);
            if(q.exec() == false)
                return false;
        }
    }
    db.close();
    return success;
}

bool DB::ProfileDelete(int indexId)
{
    return true;
}

bool DB::ProfileModifyInfo(QSharedPointer<Profile> profile, QSharedPointer<Profile> targetProfile)
{
    CreateConnection();
    QSqlQuery query;
    query.prepare(R"(
        UPDATE profiles
        SET
            profile_name = :profile_name,
            isppa       = :isppa,
            rip         = :rip,
            period      = :period,
            dc          = :dc,
            timer       = :timer,
            depth       = :depth,
            freq        = :freq,
            temp        = :temp,
            burstLen    = :burstLen,
            hcd1        = :hcd1,
            freq1       = :freq1,
            delay1      = :delay1,
            hcd2        = :hcd2,
            freq2       = :freq2,
            delay2      = :delay2,
            hcd3        = :hcd3,
            freq3       = :freq3,
            delay3      = :delay3,
            hcd4        = :hcd4,
            freq4       = :freq4,
            delay4      = :delay4
        WHERE id = :indexId
    )");

    query.bindValue(":profile_name", profile->profileName);
    query.bindValue(":isppa",      profile->isppa);
    query.bindValue(":rip",        profile->rip);
    query.bindValue(":period",     profile->period);
    query.bindValue(":dc",         profile->dc);
    query.bindValue(":timer",      profile->timer);
    query.bindValue(":depth",      profile->depth);
    query.bindValue(":freq",       profile->freq);
    query.bindValue(":temp",       profile->temp);
    query.bindValue(":voltage",    profile->voltage);
    query.bindValue(":burstLen",   profile->burstLen);
    query.bindValue(":hcd1",       profile->infoList[0]->hcd);
    query.bindValue(":freq1",      profile->infoList[0]->freq);
    query.bindValue(":delay1",     profile->infoList[0]->delay);
    query.bindValue(":hcd2",       profile->infoList[1]->hcd);
    query.bindValue(":freq2",      profile->infoList[1]->freq);
    query.bindValue(":delay2",     profile->infoList[1]->delay);
    query.bindValue(":hcd3",       profile->infoList[2]->hcd);
    query.bindValue(":freq3",      profile->infoList[2]->freq);
    query.bindValue(":delay3",     profile->infoList[2]->delay);
    query.bindValue(":hcd4",       profile->infoList[3]->hcd);
    query.bindValue(":freq4",      profile->infoList[3]->freq);
    query.bindValue(":delay4",     profile->infoList[3]->delay);
    query.bindValue(":indexId",    targetProfile->indexId);
    bool success = query.exec();
    QString logInfo = QString("UPDATE profiles set %1, where index = %2").arg(profile->GetInfo()).arg(targetProfile->indexId);
    WriteLog(logInfo);
    if (!success)
    {
        QString info =  QString("ProfileModifyInfo error2: %1").arg(query.lastError().text());
        WriteLog(info);
    }
    else
    {
        targetProfile->CopyInfo(profile.data());
    }
    db.close();
    return success;
}

bool DB::ProfileModifyInfoLIFU(QSharedPointer<ProfileLIFU> profile, QSharedPointer<ProfileLIFU> targetProfile)
{
    CreateConnection();
    QSqlQuery query(db);
    if (!db.transaction()) {
        qWarning() << "transaction failed";
        return false;
    }

    query.prepare(R"(UPDATE lifu_profiles SET profile_name = ?, dutyc= ?, period = ?, timer = ?, temp = ?, voltage = ? WHERE id = ?)");

    query.addBindValue(profile->profileName);
    query.addBindValue(profile->dutyc);
    query.addBindValue(profile->period);
    query.addBindValue(profile->timer);
    query.addBindValue(profile->temp);
    query.addBindValue(profile->voltage);
    query.addBindValue(targetProfile->indexId);

    if (!query.exec()) {
        qWarning() << query.lastError().text();
        db.rollback();
        db.close();
        return false;
    }
    query.prepare(
        "UPDATE lifu_profile_value SET value=? "
        "WHERE profile_id=? AND idx=?"
    );

    for (int idx = 0; idx < ProfileLIFU::ValueCount; ++idx) {
        query.bindValue(0, profile->values[idx]);
        query.bindValue(1, targetProfile->indexId);
        query.bindValue(2, idx);

        if (!query.exec()) {
            qWarning() << query.lastError().text();
            db.rollback();
            db.close();
            return false;
        }
    }
    if (!db.commit()) {
        qWarning() << "commit failed";
        db.close();
        return false;
    }
    targetProfile->CopyInfo(profile.data());
    QString logInfo = QString("UPDATE lifu_profiles set %1, where index = %2").arg(targetProfile->GetInfo()).arg(targetProfile->indexId);
    WriteLog(logInfo);
    db.close();
    return true;
}

QVector<QSharedPointer<Profile> > DB::ProfileGetAllInfo()
{
    QVector<QSharedPointer<Profile> > profileList;
    CreateConnection();
    QSqlQuery query;
    QString sql = QString("SELECT  * from profiles");
    query.prepare(sql);
    if (!query.exec())
    {
        QString info =  QString("ProfileGetAllInfo error: %1").arg(query.lastError().text());
        WriteLog(info);
        db.close();
        return profileList;
    }

    while (query.next())
    {
        QSharedPointer<Profile> profile = QSharedPointer<Profile>::create();
        profile->indexId = query.value("id").toInt();
        profile->profileName = query.value("profile_name").toString();
        profile->isppa = query.value("isppa").toDouble();
        profile->rip = query.value("rip").toDouble();
        profile->depth = query.value("depth").toDouble();
        profile->freq = query.value("freq").toDouble();
        profile->temp = query.value("temp").toDouble();
        profile->voltage = query.value("voltage").toDouble();
        profile->period = query.value("period").toInt();
        profile->timer = query.value("timer").toInt();
        profile->dc = query.value("dc").toInt();
        profile->burstLen = query.value("burstLen").toDouble();
        for(int i=0; i<4; i++)
        {
            profile->infoList[i]->hcd = query.value(QString("hcd%1").arg(i+1)).toDouble();
            profile->infoList[i]->freq = query.value(QString("freq%1").arg(i+1)).toDouble();
            profile->infoList[i]->delay = query.value(QString("delay%1").arg(i+1)).toDouble();
        }
        profileList.append(profile);
    }
    if(profileList.size() == 0)
    {
        QSharedPointer<Profile> profile = QSharedPointer<Profile>::create();
        if(ProfileCreate(profile, true))
        {
            profileList.append(profile);
        }
    }
    db.close();
    return profileList;
}

QVector<QSharedPointer<ProfileLIFU> > DB::ProfileGetAllInfoLIFU()
{
    QVector<QSharedPointer<ProfileLIFU> > profileList;
    CreateConnection();
    QSqlQuery query;
    QString sql = QString("SELECT  * from lifu_profiles");
    query.prepare(sql);
    if (!query.exec())
    {
        QString info =  QString("ProfileGetAllInfo error: %1").arg(query.lastError().text());
        WriteLog(info);
        db.close();
        return profileList;
    }

    while (query.next())
    {
        QSharedPointer<ProfileLIFU> profile = QSharedPointer<ProfileLIFU>::create();
        profile->indexId = query.value("id").toInt();
        profile->profileName = query.value("profile_name").toString();
        profile->dutyc = query.value("dutyc").toDouble();
        profile->temp = query.value("temp").toDouble();
        profile->voltage = query.value("voltage").toDouble();
        profile->period = query.value("period").toInt();
        profile->timer = query.value("timer").toInt();
        QSqlQuery query1;
        query1.prepare(R"(
            SELECT idx, value
            FROM lifu_profile_value
            WHERE profile_id = ?
            ORDER BY idx
        )");
        query1.addBindValue(profile->indexId);

        if (!query1.exec()) {
            qWarning() << query1.lastError().text();
            continue;
        }

        while (query1.next()) {
            int idx   = query1.value(0).toInt();
            int value = query1.value(1).toInt();

            if (idx >= 0 && idx < 128) {
                profile->values[idx] = value;
            }
        }
        profileList.append(profile);
    }
    if(profileList.size() == 0)
    {
        QSharedPointer<ProfileLIFU> profile = QSharedPointer<ProfileLIFU>::create();
        if(ProfileCreateLIFU(profile, true))
        {
            profileList.append(profile);
        }
    }
    db.close();
    return profileList;
}
