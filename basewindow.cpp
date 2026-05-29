#include "basewindow.h"
#include "logmanager.h"
#include "messageinfo.h"
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
}

uint8_t BaseWindow::GetDeviceAddr()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::LIFU_DATA);
    if(m_DataManager->GetClinicalMode() == ClinicalMode::HIFU)
        deviceAddr = static_cast<uint8_t>(DataType::HIFU_DATA);
    return deviceAddr;
}

uint32_t BaseWindow::GetValueFromQByteArray(QByteArray data, int startIndex, int len)
{
    if(data.size() < startIndex + 4)
        return 0;
    uint32_t result =
        (static_cast<uint8_t>(data[startIndex]) << 24) |
        (static_cast<uint8_t>(data[startIndex + 1]) << 16) |
        (static_cast<uint8_t>(data[startIndex + 2]) << 8) |
        static_cast<uint8_t>(data[startIndex + 3]);
    return result;
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
    if(serialMer->m_SerialPort == nullptr)
    {
        MessageInfo::ShowInformation(tr("No available port detected."));
        return;
    }
    serialPortThread = new QThread();
    serialMer->moveToThread(serialPortThread);
    serialMer->m_SerialPort->moveToThread(serialPortThread);
    serialMer->m_HeartTimer->moveToThread(serialPortThread);
    serialMer->m_SendTimeoutTimer->moveToThread(serialPortThread);
    serialMer->m_PostSendDelayTimer->moveToThread(serialPortThread);
    connect(serialPortThread, &QThread::finished, serialMer, &QObject::deleteLater);
    connect(serialMer, &SerialManager::writeLog, this, &BaseWindow::WriteCommLog);
    connect(serialMer, &SerialManager::readSerialData, this, &BaseWindow::WriteCommLog);
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
    SendCommandSystemEmit();
}

void BaseWindow::ReadDataHIFU(QByteArray data)
{

}

void BaseWindow::ReadDataLIFU(QByteArray data)
{

}

void BaseWindow::ReadDataSystem(QByteArray data)
{
    uint8_t commandType = static_cast<uint8_t>(data[3]);
    switch(commandType)
    {
    case 0x03:
        uint32_t deviceStatus = GetValueFromQByteArray(data, 7, 4);
        auto oldState = m_ConnectState;
        if(deviceStatus > static_cast<uint32_t>(ConnectState::NORMAL_OUTPUT))
            m_ConnectState = ConnectState::DISCONNECT;
        else
            m_ConnectState = static_cast<ConnectState>(deviceStatus);
        SetConnectState(m_ConnectState);
        if(oldState == ConnectState::DISCONNECT && m_ConnectState != oldState)
        {
            SendInitCommand();
        }
        uint32_t hvout = GetValueFromQByteArray(data, 11, 4);

        uint32_t temp = GetValueFromQByteArray(data, 15, 4);
        break;
    }
}

void BaseWindow::ReadSerialData(QByteArray data)
{
    uint8_t packetType = static_cast<uint8_t>(data[4]);
    if (packetType == static_cast<uint8_t>(DataType::SYSTEM_DATA)) {
        ReadDataSystem(data);
    }
    else if (packetType == static_cast<uint8_t>(DataType::HIFU_DATA)) {
        ReadDataHIFU(data);
    }
    else if (packetType == static_cast<uint8_t>(DataType::LIFU_DATA)) {
        ReadDataLIFU(data);
    }
}

void BaseWindow::Send(uint8_t cmd, uint8_t addr, uint16_t len, QByteArray data)
{
    if(m_ConnectState == ConnectState::DISCONNECT)
        return;
    emit send(cmd, addr, len, data);
}

void BaseWindow::SendCommandData4(uint8_t commandId, uint32_t value)
{
    uint8_t deviceAddr = GetDeviceAddr();
    QByteArray data = FromUint32(value);
    Send(commandId, deviceAddr, 4, data);
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

    Send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemEmit()
{
    SendCommandSystemEmitReady();
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x0E;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemEmitReady()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x18;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemHostConnectStatus(HostControlMode mode)
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x04;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    if(mode == HostControlMode::REMOTE)
        data[3] = 1;
    Send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemHostCheckStatus()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x02;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemHostCheckSN()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x18;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemModel()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x06;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    if(m_DataManager->GetClinicalMode() != ClinicalMode::HIFU)
        data[3] = 1;
    Send(commandId, deviceAddr, len, data);
}

void BaseWindow::SendCommandSystemTriggerModel()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x14;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    data[3] = static_cast<uint8_t>(m_DataManager->m_TriggerMode);
    Send(commandId, deviceAddr, len, data);
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
