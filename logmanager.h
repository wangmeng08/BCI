#pragma once

#include <qobject.h>
#include <QThread>
#include <QGlobalStatic>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QTimer>

#include "enuminfo.h"
class Protocol
{
public:
    uint16_t header;
    uint8_t index;
    uint8_t commandId;
    uint8_t deviceAddr;
    uint16_t dataLen;
    uint8_t* data;
    uint32_t crc;
    uint8_t tailer;
};

class LogManager: public QObject
{
	Q_OBJECT
public:
	friend class TestLogManager;
	static LogManager* GetInstance();

public:
	LogManager();
	~LogManager();
	friend class TestLogManager;
    QTimer *timer;

public slots:
    void WriteLog(LogType type, QString info);

private slots:
	void OnTimerHandler();

private:
	QFile logFile;
    QString GetLogContent(LogType type, QString info);
	QStringList m_LogBuffer;
};
