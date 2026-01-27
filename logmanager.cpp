#include "logmanager.h"
#include <iostream>
#include <qdir.h>

Q_GLOBAL_STATIC(LogManager, logManager);

LogManager* LogManager::GetInstance()
{
	return logManager();
}

LogManager::LogManager()
{
	QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
	QDir dir("logs");
	if (dir.exists() == false)
	{
		dir.mkdir("../logs");
	}
	// 设置日志文件名为启动时的时间
	logFile.setFileName("logs/" + currentTime + "_log.txt");

	// 打开日志文件（如果文件不存在则创建）
	if (!logFile.open(QIODevice::Append | QIODevice::Text)) 
		return;
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
	m_LogBuffer.append(content);
}

void LogManager::OnTimerHandler()
{
	if (m_LogBuffer.isEmpty())
		return;
	QTextStream out(&logFile);
	out << m_LogBuffer.join("\n");  // 将QStringList合并为一个单一字符串，并写入文件
	out.flush();  // 确保数据立即写入文件

	// 清空缓存
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
