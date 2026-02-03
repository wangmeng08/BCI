#ifndef DB_H
#define DB_H
#include <array>
#include <QObject>
#include <QSqlDatabase>
#include <QSharedPointer>
class DetailInfo{
public:
    int index = 1;
    double hcd = 0.39;
    double freq = 500;
    double delay = 0;
    DetailInfo() = default;
       DetailInfo(const DetailInfo& other) {
           index = other.index;
           hcd = other.hcd;
           freq = other.freq;
           delay = other.delay;
       }
};

class Patient{
public:
    int index = 0;
    int animalType = 0;
    int age = 1;
    double weight = 0;
    QString illness = "";
    QString name = "";
    QString GetInfo(){
        QString info = QString(
            "Patient name: %1, illness: %2, animalType: %3, age: %4, weight: %5, index: %6\n")
            .arg(name, illness,
                 QString::number(animalType),
                 QString::number(age),
                 QString::number(weight),
                 QString::number(index));
        return info;
    }
};
class BaseProfile{
public:
    int indexId = 0;
    QString profileName = "default";
    int period = 60;
    int timer = 60;
    double temp = 33.2;
    double voltage = 29.55;
};

class Profile : public BaseProfile{
public:
    double isppa = 10;
    double rip = 1;
    int dc = 50;
    double depth = 2;
    double freq = 500;
    double burstLen = 5;
    QVector<DetailInfo *> infoList;
    Profile(){
        for(int i=1; i<=4; i++)
        {
            DetailInfo *info = new DetailInfo();
            info->index = i;
            infoList.append(info);
        }
    }
    ~Profile()
     {
         qDeleteAll(infoList);
         infoList.clear();
     }
    double GetPower() const
    {
        return isppa/rip;
    }
    double GetIspta() const
    {
        return isppa * dc/1000;
    }

    void CopyInfo(const Profile* other) {
        if (!other) return;

        profileName = other->profileName;
        isppa = other->isppa;
        rip = other->rip;
        period = other->period;
        dc = other->dc;
        timer = other->timer;
        depth = other->depth;
        freq = other->freq;
        temp = other->temp;
        voltage = other->voltage;
        burstLen = other->burstLen;

        qDeleteAll(infoList);
        infoList.clear();
        for (const auto &detail : other->infoList) {
            infoList.append(new DetailInfo(*detail));
        }
    }

    QString GetInfo() const
    {
        QString infoStr = QString(
                    "ProfileName: %1, IndexId: %2, Isppa: %3, Period: %4, DC: %5, "
                    "Timer: %6, Depth: %7, Freq: %8, Temp: %9, Voltage: %10, BurstLen: %11\n"
                ).arg(profileName)
                 .arg(indexId)
                 .arg(isppa)
                 .arg(period)
                 .arg(dc)
                 .arg(timer)
                 .arg(depth)
                 .arg(freq)
                 .arg(temp)
                 .arg(voltage)
                 .arg(burstLen);

        infoStr += "DetailInfo List:\n";
        for (const auto &detail : infoList) {
            infoStr += QString("  Index: %1, HCD: %2, Freq: %3, Delay: %4\n")
                        .arg(detail->index)
                        .arg(detail->hcd)
                        .arg(detail->freq)
                        .arg(detail->delay);
        }
        return infoStr;
    }
};

class ProfileLIFU : public BaseProfile{
public:
    double dutyc = 50;
    static constexpr int ValueCount = 128;
    std::array<int, ValueCount> values;
    ProfileLIFU()
    {
        values.fill(0);
    }
    ~ProfileLIFU()
    {

    }

    void CopyInfo(const ProfileLIFU* other) {
        if (!other) return;

        profileName = other->profileName;
        period = other->period;
        timer = other->timer;
        dutyc = other->dutyc;
        temp = other->temp;
        voltage = other->voltage;

        for(int i=0; i<ValueCount; i++)
        {
            values[i] = other->values[i];
        }
    }

    QString GetInfo() const
    {
        QString infoStr = QString(
                    "ProfileName: %1, IndexId: %2, dutyc: %3, Period: %4, timer: %5, "
                    "temp: %6, voltage: %7\n"
                ).arg(profileName)
                 .arg(indexId)
                 .arg(dutyc)
                 .arg(period)
                 .arg(timer)
                 .arg(temp)
                 .arg(voltage);

        infoStr += "DetailInfo List: ";
        for(int i=0; i<ValueCount; i++)
        {
            infoStr += QString("%1, ").arg(values[i]);
        }
        infoStr + "\n";
        return infoStr;
    }
};

class ProfileLIFU4 : public BaseProfile{
public:
    double dutyc = 50;
    static constexpr int ValueCount = 4;
    std::array<int, ValueCount> values;
    ProfileLIFU4()
    {
        values.fill(0);
    }
    ~ProfileLIFU4()
    {

    }

    void CopyInfo(const ProfileLIFU4* other) {
        if (!other) return;

        profileName = other->profileName;
        period = other->period;
        timer = other->timer;
        dutyc = other->dutyc;
        temp = other->temp;
        voltage = other->voltage;

        for(int i=0; i<ValueCount; i++)
        {
            values[i] = other->values[i];
        }
    }

    QString GetInfo() const
    {
        QString infoStr = QString(
                    "ProfileName: %1, IndexId: %2, dutyc: %3, Period: %4, timer: %5, "
                    "temp: %6, voltage: %7\n"
                ).arg(profileName)
                 .arg(indexId)
                 .arg(dutyc)
                 .arg(period)
                 .arg(timer)
                 .arg(temp)
                 .arg(voltage);

        infoStr += "DetailInfo List: ";
        for(int i=0; i<ValueCount; i++)
        {
            infoStr += QString("%1, ").arg(values[i]);
        }
        infoStr + "\n";
        return infoStr;
    }
};

class Report{
public:
    int index = 0;
    int patientIndex = 0;
    QString fileName = "";
    QString reportTime = "";
    QString GetInfo(){
        return QString("PatientIndex: %1, fileName: %2").arg(patientIndex).arg(fileName);
    }
};

class DB : public QObject
{
    Q_OBJECT
public:
    explicit DB(QObject *parent = nullptr);
    static DB *GetInstance();

    bool PatientCreate(QSharedPointer<Patient> patient);
    bool PatientDelete(int indexId);

    bool ProfileCreate(QSharedPointer<Profile> profile, bool isDefault = false);
    bool ProfileCreateLIFU(QSharedPointer<ProfileLIFU> profile, bool isDefault = false);
    bool ProfileCreateLIFU4(QSharedPointer<ProfileLIFU4> profile, bool isDefault = false);
    bool ProfileDelete(int indexId);
    bool ProfileModifyInfo(QSharedPointer<Profile> profile, QSharedPointer<Profile> targetProfile);
    bool ProfileModifyInfoLIFU(QSharedPointer<ProfileLIFU> profile, QSharedPointer<ProfileLIFU> targetProfile);
    bool ProfileModifyInfoLIFU4(QSharedPointer<ProfileLIFU4> profile, QSharedPointer<ProfileLIFU4> targetProfile);

    bool ReportCreate(QSharedPointer<Report> report);

    void ReportGetAllInfo(QHash<int, QVector<QSharedPointer<Report>>> &reportList);
    void PatientGetAllInfo(QVector<QSharedPointer<Patient>> &patientList);

    void ProfileGetAllInfo(QVector<QSharedPointer<Profile>> &profileList);
    void ProfileGetAllInfoLIFU(QVector<QSharedPointer<ProfileLIFU>> &profileList);
    void ProfileGetAllInfoLIFU4(QVector<QSharedPointer<ProfileLIFU4>> &profileList);
signals:

private:

  private:
    bool CreateConnection();

    void CreateTableLIFUProfile();
    void CreateTableLIFUProfile4();
    void CreateTableLIFUProfileValue();

    void CreateTableProfile();

    void CreateTablePatient();

    void CreateTableReport();

    void InitDataBase();
    void WriteLog(QString info);
    QSqlDatabase db;
};

#endif // DB_H
