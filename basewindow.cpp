#include "basewindow.h"
#include "logmanager.h"
#include "serialmanager1.h"
#include "eventmanager.h"

BaseWindow::BaseWindow(QWidget *parent)
    : QMainWindow{parent}
{
    qRegisterMetaType<LogType>("LogType");
    qRegisterMetaType<TriggerMode>("TriggerMode");
    InitLogManager();
    InitSerialManager();
    m_DB = DB::GetInstance();
    m_DataManager = DataManager::GetInstance();
    connect(EventManager::GetInstance(), &EventManager::writeLog, this, &BaseWindow::writeLog);
}

void BaseWindow::InitDatabase()
{

}

void BaseWindow::InitLogManager()
{
    LogManager *logMgr = LogManager::GetInstance();
    logThread = new QThread();
    logMgr->moveToThread(logThread);
    logMgr->timer->moveToThread(logThread);
    connect(this, &BaseWindow::writeLog, logMgr, &LogManager::WriteLog);
    connect(logThread, &QThread::finished, logMgr, &QObject::deleteLater);
    logThread->start();
}

void BaseWindow::InitSerialManager()
{
    SerialManager *serialMer = SerialManager::GetInstance();
    serialPortThread = new QThread();
    serialMer->moveToThread(serialPortThread);
    serialMer->mSerialPort->moveToThread(serialPortThread);
    serialMer->heartTimer->moveToThread(serialPortThread);
    connect(serialPortThread, &QThread::finished, serialMer, &QObject::deleteLater);
    connect(serialMer, &SerialManager::writeLog, this, &BaseWindow::WriteCommLog);
    connect(this, &BaseWindow::heartTimerStart, serialMer, &SerialManager::HeartTimerStop);
    connect(this, &BaseWindow::heartTimerStop, serialMer, &SerialManager::HeartTimerStop);
    connect(this, &BaseWindow::serialPortOpen, serialMer, &SerialManager::SerialPortOpen);
    serialPortThread->start();
    emit serialPortOpen();
}

void BaseWindow::SetConnectState(ConnectState state)
{
    int index = (int)state;
    auto info = m_ConnectDesList[index];
    auto qss = m_ConnectQss[index];
    auto label = GetConnectLabel();
    label->setText(info);
    label->setStyleSheet(qss);
}

void BaseWindow::SetEmitState(EmitState state)
{
    m_State = state;
    int index = (int)state;
    auto info = m_EmitDesList[index];
    auto qss = m_EmitQss[index];
    auto label = GetEmitLabel();
    label->setText(tr("Emit state: %1").arg(info));
    label->setStyleSheet(qss);
}

void BaseWindow::WriteCommLog(QString info)
{
    emit writeLog(LogType::COMM, info);
}

void BaseWindow::WriteDBLog(QString info)
{
    emit writeLog(LogType::DB, info);
}
