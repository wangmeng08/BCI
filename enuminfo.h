#ifndef ENUMINFO_H
#define ENUMINFO_H
#include <QMetaType>

enum class ClinicalMode{
    HIFU = 1,
    LIFU128,
    LIFU4
};

enum class ConnectState{
    DISCONNECT = 0,
    STANDBY,
    OVERHEAT = 2,
    OVERCURRENT = 3,
    NORMAL_OUTPUT = 4
};

enum class DataType : uint8_t
{
    HIFU_DATA = 1,
    LIFU_DATA = 2,
    SYSTEM_DATA = 4
};

enum class EmitState{
    IDLE = 0,
    ON,
    ERROR
};

enum class HostControlMode{
    LOCAL = 0,
    REMOTE
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
    Choose = 0,
    PatientManager,
    HIFU,
    LIFU,
    LIFU4
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
