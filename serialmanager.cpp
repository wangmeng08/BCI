#include "serialmanager.h"
#include <iostream>

Q_GLOBAL_STATIC(SerialManager, serialManager);

SerialManager* SerialManager::GetInstance()
{
	return serialManager();
}

SerialManager::SerialManager()
{
	InitSerialPort();

	heartTimer = new QTimer;
	connect(heartTimer, &QTimer::timeout, this, &SerialManager::OnHeartTimeBeat);
	heartTimer->stop();

    m_SendTimeoutTimer = new QTimer(this);
    m_SendTimeoutTimer->setSingleShot(true);
    m_SendTimeoutTimer->setInterval(200);
    connect(m_SendTimeoutTimer, &QTimer::timeout, this, &SerialManager::OnSendTimeout);
}

SerialManager::~SerialManager() {
    if (m_SerialPort) {
        if (m_SerialPort->isOpen())
            m_SerialPort->close();
        m_SerialPort->deleteLater();
        m_SerialPort = nullptr;
    }
}

void SerialManager::HeartTimerStart()
{

}

void SerialManager::HeartTimerStop()
{

}

void SerialManager::SerialPortClose()
{

}

void SerialManager::SerialPortOpen()
{

}

bool SerialManager::ParseSerialData(QByteArray &packet)
{
    while (m_Buffer.size() >= 11)
    {
        int pos = -1;
        for (int i = 0; i < m_Buffer.size() - 1; ++i)
        {
            if ((uint8_t)m_Buffer[i] == 0x5A &&
                (uint8_t)m_Buffer[i + 1] == 0xA5)
            {
                pos = i;
                break;
            }
        }

        if (pos < 0)
        {
            m_Buffer.clear();
            return false;
        }

        if (pos > 0)
            m_Buffer.remove(0, pos);

        if (m_Buffer.size() < 7) return false;

        uint16_t len =
            ((uint8_t)m_Buffer[5] << 8) |
            ((uint8_t)m_Buffer[6]);
        if (len == 0 || len > 1024)
        {
            m_Buffer.remove(0, 1);
            continue;
        }
        int totalLen = 7 + len + 4 + 1;

        if (m_Buffer.size() < totalLen)
            return false;
        QByteArray one = m_Buffer.left(totalLen);

        uint8_t tail = (uint8_t)one.at(totalLen - 1);
        if (tail != 0xDD)
        {
            m_Buffer.remove(0, 1);
            continue;
        }

        uint32_t recvCrc =
                ((uint8_t)one[7 + len] << 24) |
                ((uint8_t)one[7 + len + 1] << 16) |
                ((uint8_t)one[7 + len + 2] << 8) |
                ((uint8_t)one[7 + len + 3]);
        QByteArray crcRange = one.left(7 + len);

        if (Crc32(crcRange) != recvCrc)
        {
            m_Buffer.remove(0, 1);
            continue;
        }

        packet = one;
        m_Buffer.remove(0, totalLen);
        return true;
    }
    return false;
}

void SerialManager::OnSerialDataRead()
{
    m_Buffer.append(m_SerialPort->readAll());
    QByteArray packet;
    while (ParseSerialData(packet))
    {
        emit readSerialData(packet);
        WriteQByteArrayLog(packet);

        if (m_IsSending)
        {
            m_SendTimeoutTimer->stop();
            m_IsSending = false;
            TrySendNext();
        }
    }
}

void SerialManager::InitSerialPort()
{
    auto portList = QSerialPortInfo::availablePorts();
    if(portList.count() == 0)
    {
        emit writeLog("No available port name detected.");
        return;
    }
    auto portName = portList[0].portName();
    m_SerialPort = new QSerialPort(this);
    m_SerialPort->setPortName(portName);
    m_SerialPort->setBaudRate(QSerialPort::Baud115200);
    m_SerialPort->setDataBits(QSerialPort::Data8);
    m_SerialPort->setParity(QSerialPort::NoParity);
    m_SerialPort->setStopBits(QSerialPort::OneStop);
    m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);
    connect(m_SerialPort, &QSerialPort::readyRead, this, &SerialManager::OnSerialDataRead);
    if (!m_SerialPort->open(QIODevice::ReadWrite))
	{
        emit writeLog("open port failed");
		return;
    }
}

void SerialManager::Send(uint8_t cmd, uint8_t addr, uint16_t len, QByteArray data)
{
    //if (!m_SerialPort || !m_SerialPort->isOpen())
      //  return;
    QByteArray packet = PackPacket(cmd, addr, len, data);
    m_SendQueue.enqueue(packet);
    TrySendNext();
}

void SerialManager::Test(uint8_t cmd)
{
    emit writeLog("Test");
}

void SerialManager::TrySendNext()
{
    emit writeLog("TrySendNext");
    if (m_IsSending)
        return;
    if (m_SendQueue.isEmpty())
        return;
    if (!m_SerialPort || !m_SerialPort->isOpen())
        return;

    QByteArray packet = m_SendQueue.dequeue();
    m_IsSending = true;
    m_SerialPort->write(packet);
    WriteQByteArrayLog(packet);
    m_SendTimeoutTimer->start();
}

void SerialManager::OnSendTimeout()
{
    m_IsSending = false;
    TrySendNext();
}

void SerialManager::WriteQByteArrayLog(QByteArray data, bool isReceiveInfo)
{

    QString  info = "Send: ";
    if(isReceiveInfo)
        info = "Receive: ";
    info = info + data.toHex(' ');
    emit writeLog(info);
}

QByteArray SerialManager::PackPacket(uint8_t cmd, uint8_t addr, uint16_t len, const QByteArray &data)
{
    QByteArray packet;

    packet.append((char)0x5A);
    packet.append((char)0xA5);
    packet.append(m_Index);
    m_Index = (m_Index + 1) & 0xFF;
    packet.append(cmd);
    packet.append(addr);

    packet.append((char)((len >> 8) & 0xFF));
    packet.append((char)(len & 0xFF));

    packet.append(data);

    QByteArray crcRange = packet;
    uint32_t crc = Crc32(crcRange);
    packet.append((char)((crc >> 24) & 0xFF));
    packet.append((char)((crc >> 16) & 0xFF));
    packet.append((char)((crc >> 8) & 0xFF));
    packet.append((char)(crc & 0xFF));

    uint8_t tail = 0xDD;
    packet.append(tail);

    return packet;
}

uint32_t SerialManager::Crc32(const QByteArray &data)
{
    uint32_t crc = 0xFFFFFFFF;
    for (auto byte : data)
    {
     crc ^= (uint8_t)byte << 24;
     for (int i = 0; i < 8; i++)
     {
         if (crc & 0x80000000)
             crc = (crc << 1) ^ 0x04C11DB7;
         else
             crc <<= 1;
     }
    }
    return crc;
}

void SerialManager::OnHeartTimeBeat()
{

}

