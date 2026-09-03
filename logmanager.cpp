#include "logmanager.h"
#include <iostream>
#include <qdir.h>
#include <QCoreApplication>

Q_GLOBAL_STATIC(LogManager, logManager);

LogManager* LogManager::GetInstance()
{
	return logManager();
}

LogManager::LogManager()
{
	QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    const QString logDirectory =
            QCoreApplication::applicationDirPath() + QStringLiteral("/logs");
    QDir().mkpath(logDirectory);
    logFile.setFileName(logDirectory + QStringLiteral("/") +
                        currentTime + QStringLiteral("_log.txt"));

	// 打开日志文件（如果文件不存在则创建）
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qDebug().noquote() << QStringLiteral("Open log file failed: %1")
                              .arg(logFile.fileName());
		return;
    }
    qDebug().noquote() << QStringLiteral("Log file: %1").arg(logFile.fileName());
	timer = new QTimer;
	connect(timer, &QTimer::timeout, this, &LogManager::OnTimerHandler);
	timer->start(5000);
}

LogManager::~LogManager() {
	OnTimerHandler();
	timer->stop();
	delete timer;
}

void LogManager::WriteLog(LogType type, QString info)
{
	QString content = GetLogContent(type, info);
    qDebug().noquote() << content;
    QTextStream out(&logFile);
    out << content << "\n";
    out.flush();
}

QString LogManager::LogFilePath() const
{
    return logFile.fileName();
}

void LogManager::OnTimerHandler()
{
	if (m_LogBuffer.isEmpty())
		return;
	QTextStream out(&logFile);
    out << m_LogBuffer.join("\n");
    out << ("\n");
    out.flush();

	m_LogBuffer.clear();
}

QString LogManager::GetLogContent(LogType type, QString info)
{
	QString totleInfo = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	switch (type)
	{
    case LogType::INFO:
		totleInfo.append(": info: ");
		break;
    case LogType::WARNING:
		totleInfo.append(": warning: ");
		break;
    case LogType::ERROR:
        totleInfo.append(": error: ");
        break;
    case LogType::COMM:
        totleInfo.append(": comm: ");
        break;
    case LogType::DB:
        totleInfo.append(": db: ");
        break;
	default:
		totleInfo.append(": other: ");
		break;
	}
	totleInfo.append(info);
	return totleInfo;
}
