#include "basewindow.h"
#include "logmanager.h"
#include "messageinfo.h"
#include "serialmanager.h"
#include "eventmanager.h"

#include <QApplication>
#include <QMetaObject>
#include <QSerialPortInfo>

namespace {
QThread *s_serialPortThread = nullptr;
}

BaseWindow::BaseWindow(QWidget *parent)
    : QWidget{parent}
{
    m_DB = DB::GetInstance();
    m_DataManager = DataManager::GetInstance();
    InitSerialManager();
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
    Q_UNUSED(len);
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
    if (!s_serialPortThread) {
        s_serialPortThread = new QThread(qApp);
        serialMer->moveToThread(s_serialPortThread);
        QObject::connect(qApp, &QApplication::aboutToQuit, []() {
            SerialManager *serialMer = SerialManager::GetInstance();
            if (s_serialPortThread && s_serialPortThread->isRunning()) {
                const Qt::ConnectionType connectionType =
                        QThread::currentThread() == serialMer->thread()
                        ? Qt::DirectConnection
                        : Qt::BlockingQueuedConnection;
                QMetaObject::invokeMethod(serialMer, "SerialPortClose",
                                          connectionType);
                s_serialPortThread->quit();
                s_serialPortThread->wait();
                s_serialPortThread = nullptr;
            }
        });
        s_serialPortThread->start();
    }
    serialPortThread = s_serialPortThread;
    connect(serialMer, &SerialManager::writeLog, this, &BaseWindow::WriteCommLog);
    connect(serialMer, &SerialManager::readSerialData, this, &BaseWindow::ReadSerialData);
    connect(serialMer, &SerialManager::commandAccepted, this, &BaseWindow::OnCommandAccepted);
    connect(serialMer, &SerialManager::commandRejected, this, &BaseWindow::OnCommandRejected);
    connect(serialMer, &SerialManager::serialPortStateChanged, this, [this](bool isOpen) {
        if (isOpen) {
            if (m_ConnectState == ConnectState::DISCONNECT) {
                m_ConnectState = ConnectState::STANDBY;
                SetConnectState(m_ConnectState);
            }
        }
        else {
            m_EmitStartPending = false;
            m_ConnectState = ConnectState::DISCONNECT;
            SetConnectState(m_ConnectState);
        }
    });
    connect(this, &BaseWindow::heartTimerStart, serialMer, &SerialManager::HeartTimerStart);

    connect(this, &BaseWindow::heartTimerStop, serialMer, &SerialManager::HeartTimerStop);
    connect(this, &BaseWindow::send, serialMer, &SerialManager::Send);
    connect(this, &BaseWindow::test, serialMer, &SerialManager::Test);
    connect(this, &BaseWindow::serialPortClose, serialMer, &SerialManager::SerialPortClose);
    connect(this, &BaseWindow::serialPortOpen, serialMer, &SerialManager::SerialPortOpen);
    emit serialPortOpen();
}


void BaseWindow::OnClickOff()
{
    OnSonicStopped();
    m_EmitStartPending = false;
    SendCommandSystemStop();
    SetEmitState(EmitState::IDLE);
    UpdateBtnState();
    EmitTimerStop();
}

void BaseWindow::OnClickOn()
{
    if (m_EmitStartPending)
        return;
    if (!IsSerialAvailable()) {
        WriteCommLog(QStringLiteral("[MCU] serial port unavailable, opening serial port only"));
        emit serialPortClose();
        emit serialPortOpen();
        return;
    }
    if (!IsSerialOpen() || m_ConnectState == ConnectState::DISCONNECT) {
        WriteCommLog(QStringLiteral("[MCU] serial disconnected, opening serial port only"));
        emit serialPortOpen();
        return;
    }
    WriteCommLog(QStringLiteral("[MCU] configure and start requested"));
    m_EmitStartPending = true;
    SendInitCommand();
    SendCommandSystemEmit();
}

void BaseWindow::ReadDataHIFU(QByteArray data)
{
    Q_UNUSED(data);
}

void BaseWindow::ReadDataLIFU(QByteArray data)
{
    Q_UNUSED(data);
}

void BaseWindow::ReadDataSystem(QByteArray data)
{
    uint8_t commandType = static_cast<uint8_t>(data[3]);
    switch(commandType)
    {
    case 0x03:
        if (data.size() < 11)
            return;
        uint32_t deviceStatus = GetValueFromQByteArray(data, 7, 4);
        if(deviceStatus > static_cast<uint32_t>(ConnectState::NORMAL_OUTPUT))
            m_ConnectState = ConnectState::STANDBY;
        else
            m_ConnectState = static_cast<ConnectState>(deviceStatus);
        SetConnectState(m_ConnectState);
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


void BaseWindow::OnCommandAccepted(uint8_t commandId, uint8_t addr)
{

    if (!isVisible())
        return;
    if (commandId != 0x0E || addr != static_cast<uint8_t>(DataType::SYSTEM_DATA))
        return;

    OnSonicStarted(addr);

    if (!m_EmitStartPending)
        return;

    m_EmitStartPending = false;
    SetEmitState(EmitState::ON);
    UpdateBtnState();
    EmitTimerStart();

}

void BaseWindow::OnSonicStarted(uint8_t addr)
{
    Q_UNUSED(addr);
}

void BaseWindow::OnSonicStopped()
{
}

void BaseWindow::OnCommandRejected(uint8_t commandId, uint8_t addr)
{
    if (!isVisible())
        return;
    if (commandId != 0x0E || addr != static_cast<uint8_t>(DataType::SYSTEM_DATA))
        return;

    m_EmitStartPending = false;
    SetEmitState(EmitState::ERROR);
    UpdateBtnState();
}

void BaseWindow::Send(uint8_t cmd, uint8_t addr, uint16_t len,
                      QByteArray data, const QString &description)
{
    emit send(cmd, addr, len, data, description);
}

void BaseWindow::SendCommandData4(uint8_t commandId, uint32_t value,
                                  const QString &description)
{
    uint8_t deviceAddr = GetDeviceAddr();
    QByteArray data = FromUint32(value);
    Send(commandId, deviceAddr, 4, data, description);
}

void BaseWindow::SendCommandSetEmitTime(uint32_t value)
{
    SendCommandData4(0x10, value,
                     QStringLiteral("set Timer=%1 ms").arg(value));
}

void BaseWindow::SendCommandSetFrequency(uint32_t value)
{
    if (value > 3000 || value < 250)
        return;
    SendCommandData4(0x08, value,
                     QStringLiteral("set Frequency=%1 kHz").arg(value));
}

void BaseWindow::SendCommandSetHvout(double value)
{
    if (value < 0.0 || value > 655.35)
        return;
    const uint32_t encodedValue = static_cast<uint32_t>(qRound(value * 100.0));
    SendCommandData4(0x04, encodedValue,
                     QStringLiteral("set HVOut=%1 V").arg(value));
}

void BaseWindow::SendCommandSetIsppa(double value)
{
    const double ratioSPPA = 10000.0;
    if (value < 0.0)
        return;
    const uint32_t encodedValue = static_cast<uint32_t>(qRound(value * ratioSPPA));
    SendCommandData4(0x14, encodedValue,
                     QStringLiteral("set Isppa=%1 W/cm2 encoded=%2")
                     .arg(value).arg(encodedValue));
}

void BaseWindow::SendCommandSetPD(uint32_t value)
{
    if (value > 100000 || value < 1)
        return;
    SendCommandData4(0x12, value,
                     QStringLiteral("set PD=%1 us").arg(value));
}

void BaseWindow::SendCommandSetPri(uint32_t value)
{
    if (value > 1000 || value < 1)
        return;
    SendCommandData4(0x0C, value,
                     QStringLiteral("set PRI=%1 ms").arg(value));
}

void BaseWindow::SendCommandSetChannelSwitch(int channelCount)
{
    channelCount = qBound(0, channelCount, 128);
    QByteArray data(16, 0x00);
    for (int channel = 0; channel < channelCount; ++channel) {
        const int byteIndex = channel / 8;
        const int bitIndex = channel % 8;
        data[byteIndex] = static_cast<char>(
                    static_cast<uint8_t>(data[byteIndex]) | (1u << bitIndex));
    }
    Send(0x06, GetDeviceAddr(), static_cast<uint16_t>(data.size()), data,
         QStringLiteral("set ChannelSwitch count=%1 mask=%2")
         .arg(channelCount).arg(QString(data.toHex(' '))));
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

    Send(commandId, deviceAddr, len, data,
         QStringLiteral("set ChannelDelays count=%1").arg(delays.size()));
}

void BaseWindow::SendCommandSystemEmit()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x0E;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data,
         QStringLiteral("start sonic addr=0x%1")
         .arg(deviceAddr, 2, 16, QLatin1Char('0')).toUpper());
}

void BaseWindow::SendCommandSystemStop()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x0F;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data,
         QStringLiteral("stop sonic"));
}

void BaseWindow::SendCommandSystemEmitReady()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x18;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data,
         QStringLiteral("emission ready"));
}

void BaseWindow::SendCommandSystemHostConnectStatus(HostControlMode mode)
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x04;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    if(mode == HostControlMode::REMOTE)
        data[3] = 1;
    Send(commandId, deviceAddr, len, data,
         QStringLiteral("set host control mode=%1").arg(static_cast<int>(mode)));
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
    Send(commandId, deviceAddr, len, data,
         QStringLiteral("query system SN"));
}

void BaseWindow::SendCommandSystemModel()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x06;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Q_UNUSED(deviceAddr);
    Q_UNUSED(commandId);
    Q_UNUSED(len);
    Q_UNUSED(data);
    if(m_DataManager->GetClinicalMode() != ClinicalMode::HIFU)
        data[3] = 1;
    //Send(commandId, deviceAddr, len, data,
    //     QStringLiteral("set clinical mode=%1")
    //     .arg(m_DataManager->GetClinicalMode() == ClinicalMode::HIFU
    //          ? QStringLiteral("HIFU") : QStringLiteral("LIFU")));
}

void BaseWindow::SendCommandSystemTriggerModel()
{
    uint8_t deviceAddr = static_cast<uint8_t>(DataType::SYSTEM_DATA);
    uint8_t commandId = 0x14;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    data[3] = static_cast<uint8_t>(m_DataManager->m_TriggerMode);
    Send(commandId, deviceAddr, len, data,
         QStringLiteral("set trigger mode=%1")
         .arg(static_cast<int>(m_DataManager->m_TriggerMode)));
}

void BaseWindow::SetConnectState(ConnectState state)
{
    int index = (int)state;
    if (index < 0 || index >= m_ConnectDesList.size())
        index = static_cast<int>(ConnectState::DISCONNECT);
    auto info = m_ConnectDesList[index];
    auto qss = m_ConnectQss[index];
    auto label = GetConnectLabel();
    label->setText(info);
    label->setStyleSheet(qss);
    auto stateIcon = GetStateIcon();
    const int iconIndex = state == ConnectState::DISCONNECT ? 0 : 1;
    stateIcon->setStyleSheet(m_ConnectIconQss[iconIndex]);
}

bool BaseWindow::IsSerialOpen() const
{
    SerialManager *serialMer = SerialManager::GetInstance();
    return serialMer->m_SerialPort && serialMer->m_SerialPort->isOpen();
}

bool BaseWindow::IsSerialAvailable() const
{
    SerialManager *serialMer = SerialManager::GetInstance();
    if (!serialMer->m_SerialPort || !serialMer->m_SerialPort->isOpen())
        return false;
    const QString portName = serialMer->m_SerialPort->portName();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        if (port.portName().compare(portName, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

void BaseWindow::SyncConnectStateFromSerial()
{
    m_ConnectState = IsSerialAvailable() ? ConnectState::STANDBY : ConnectState::DISCONNECT;
    SetConnectState(m_ConnectState);
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
