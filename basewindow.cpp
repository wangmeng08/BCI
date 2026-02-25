#include "basewindow.h"
#include "logmanager.h"
#include "serialmanager.h"
#include "eventmanager.h"

BaseWindow::BaseWindow(QWidget *parent)
    : QWidget{parent}
{
    InitSerialManager();
    m_DB = DB::GetInstance();
    m_DataManager = DataManager::GetInstance();
    m_Timer = new QTimer(this);
    m_Timer->setInterval(1000);
    connect(m_Timer, &QTimer::timeout, this, &BaseWindow::EmitTimerJump);
}

void BaseWindow::EmitTimerJump()
{
    m_CurrentTime--;
    SetTimerInfo();
}

void BaseWindow::EmitTimerStart()
{
    m_CurrentTime = m_DataManager->GetEmitTime();
    m_Timer->start();
    SetTimerInfo();
}

void BaseWindow::EmitTimerStop()
{
    m_Timer->stop();
    m_CurrentTime = m_DataManager->GetEmitTime();
    SetTimerInfo();
}

void BaseWindow::InitDatabase()
{

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


void BaseWindow::OnClickOff()
{
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
    EmitTimerStop();
}

void BaseWindow::OnClickOn()
{
    SetEmitState(EmitState::ON);
    UpdateBtnState();
    EmitTimerStart();
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

void BaseWindow::SetLineEditState(QLineEdit *edit, bool enable)
{
    edit->setEnabled(enable);
    SetUnderline(edit, enable);
}

void BaseWindow::SetUnderline(QLineEdit *edit, bool enable)
{
    QFont font = edit->font();
    font.setUnderline(enable);
    edit->setFont(font);
}

void BaseWindow::WriteCommLog(QString info)
{
    emit EventManager::GetInstance()->writeLog(LogType::COMM, info);
}
