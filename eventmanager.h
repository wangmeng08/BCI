#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <QObject>
#include "enuminfo.h"

class EventManager : public QObject
{
    Q_OBJECT
public:
    explicit EventManager(QObject *parent = nullptr);
    static EventManager* GetInstance();
signals:
    void writeLog(LogType type, QString info);
};

#endif // EVENTMANAGER_H
