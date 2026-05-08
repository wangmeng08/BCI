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
    m_Timer->setInterval(m_timerIntervalMs);
    connect(m_Timer, &QTimer::timeout, this, &BaseWindow::EmitTimerJump);
    SendCommandSystemModel();
}

uint8_t BaseWindow::GetDeviceAddr()
{
    uint8_t deviceAddr = 2;
    if(m_DataManager->GetClinicalMode() == ClinicalMode::HIFU)
        deviceAddr = 1;
    return deviceAddr;
}

void BaseWindow::EmitTimerJump()
{
    m_CurrentTime = m_CurrentTime - m_timerIntervalMs;
    if(m_CurrentTime <= 0)
    {
        OnClickOff();
        m_CurrentTime = 0;
    }
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
}

void BaseWindow::InitDatabase()
{

}

void BaseWindow::InitSerialManager()
{
    SerialManager *serialMer = SerialManager::GetInstance();
    serialPortThread = new QThread();
    serialMer->moveToThread(serialPortThread);
    serialMer->m_SerialPort->moveToThread(serialPortThread);
    serialMer->m_SendTimeoutTimer->moveToThread(serialPortThread);
    connect(serialPortThread, &QThread::finished, serialMer, &QObject::deleteLater);
    connect(serialMer, &SerialManager::writeLog, this, &BaseWindow::WriteCommLog);
    connect(this, &BaseWindow::heartTimerStart, serialMer, &SerialManager::HeartTimerStop);
    connect(this, &BaseWindow::heartTimerStop, serialMer, &SerialManager::HeartTimerStop);
    connect(this, &BaseWindow::send, serialMer, &SerialManager::Send);
    connect(this, &BaseWindow::test, serialMer, &SerialManager::Test);
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

void BaseWindow::SendCommandData4(uint8_t commandId, uint32_t value)
{
    uint8_t deviceAddr = GetDeviceAddr();
    QByteArray data = FromUint32(value);
    emit send(commandId, deviceAddr, 4, data);
}

void BaseWindow::SendCommandSetEmitTime(uint32_t value)
{
    if (value > 1800000 || value < 100)
        return;
    uint8_t commandId = 0x10;
    SendCommandData4(commandId, value);
}

void BaseWindow::SendCommandSetFrequency(uint32_t value)
{
    if (value > 3000 || value < 250)
        return;
    uint8_t commandId = 0x08;
    SendCommandData4(commandId, value);
}

void BaseWindow::SendCommandSetHvout(uint32_t value)
{
    if (value > 6000)
        return;
    uint8_t commandId = 0x04;
    SendCommandData4(commandId, value*100);
}

void BaseWindow::SendCommandSetPD(uint32_t value)
{
    if (value > 100000 || value < 1)
        return;
    uint8_t commandId = 0x12;
    SendCommandData4(commandId, value);
}

void BaseWindow::SendCommandSetPri(uint32_t value)
{
    if (value > 1000 || value < 1)
        return;
    uint8_t commandId = 0x0C;
    SendCommandData4(commandId, value);
}

void BaseWindow::SendCommandSetChannelDelay(const QVector<uint32_t> &delays)
{
    uint8_t deviceAddr = GetDeviceAddr();
    uint8_t commandId = 0x0A;
    uint16_t len = delays.length() * 4;
    QByteArray data;
    for(auto delay : delays)
    {
        if (delay > 40955)
            delay = 40955;
        data.append(char((delay >> 24) & 0xFF));
        data.append(char((delay >> 16) & 0xFF));
        data.append(char((delay >> 8) & 0xFF));
        data.append(char(delay & 0xFF));
    }

    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemEmit()
{
    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x0E;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemEmitReady()
{
    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x18;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemHostConnectStatus(HostControlMode mode)
{
    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x04;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    if(mode == HostControlMode::REMOTE)
        data[3] = 1;
    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemHostCheckStatus()
{
    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x02;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemHostCheckSN()
{
    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x18;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemModel()
{
    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x06;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    if(m_DataManager->GetClinicalMode() != ClinicalMode::HIFU)
        data[3] = 1;
    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemTriggerModel()
{
    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x14;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    data[3] = static_cast<uint8_t>(m_DataManager->m_TriggerMode);
    emit send(commandId, deviceAddr, len, data);
}

void BaseWindow::SetConnectState(ConnectState state)
{
    int index = (int)state;
    auto info = m_ConnectDesList[index];
    auto qss = m_ConnectQss[index];
    auto label = GetConnectLabel();
    label->setText(info);
    label->setStyleSheet(qss);
    auto stateIcon = GetStateIcon();
    stateIcon->setStyleSheet(m_ConnectIconQss[index]);
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

QByteArray BaseWindow::FromUint32(uint32_t value)
{
    QByteArray data;

    data.append(char((value >> 24) & 0xFF));
    data.append(char((value >> 16) & 0xFF));
    data.append(char((value >> 8) & 0xFF));
    data.append(char(value & 0xFF));
    return data;
}
