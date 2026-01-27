#ifndef DB_H
#define DB_H

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

class Profile{
public:
    int indexId = 0;
    QString profileName = "default";
    double isppa = 10;
    double rip = 1;
    int period = 60;
    int dc = 50;
    int timer = 60;
    double depth = 2;
    double freq = 500;
    double temp = 33.2;
    double voltage = 29.55;
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

class DB : public QObject
{
    Q_OBJECT
public:
    explicit DB(QObject *parent = nullptr);
    static DB *GetInstance();

    bool ProfileCreate(QSharedPointer<Profile> profile, bool isDefault = false);

    bool ProfileDelete(int indexId);
    bool ProfileModifyInfo(QSharedPointer<Profile> profile, QSharedPointer<Profile> targetProfile);

    QVector<QSharedPointer<Profile>> ProfileGetAllInfo();
signals:

private:

  private:
    bool CreateConnection();

    void CreateTableProfile();

    void InitDataBase();
    void WriteLog(QString info);
    QSqlDatabase db;
};

#endif // DB_H
