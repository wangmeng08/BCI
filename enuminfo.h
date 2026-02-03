#ifndef ENUMINFO_H
#define ENUMINFO_H
#include <QMetaType>

enum class ClinicalMode{
    HIFU = 0,
    LIFU
};

enum class ConnectState{
    DISCONNECT = 0,
    CONNECT
};

enum class EmitState{
    IDLE = 0,
    ON,
    ERROR
};

enum class LogType
{
    INFO = 0,
    WARNING,
    ERROR,
    DB,
    COMM
};

enum class MessageType{
    TIPS = 0,
    WARNING,
    QUESTION
};

enum class Page{
    PatientManager = 0,
    HIFU,
    LIFU
};

enum class TriggerMode{
    INTER = 0,
    EXTRE,
    EXTFE,
    EXTHL
};


Q_DECLARE_METATYPE(LogType);
Q_DECLARE_METATYPE(Page);
Q_DECLARE_METATYPE(TriggerMode);

#endif // ENUMINFO_H
