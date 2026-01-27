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
