#include "serialmanager.h"
#include <algorithm>

namespace {

int PortPriority(const QSerialPortInfo &port)
{
    const QString text = QStringLiteral("%1 %2 %3 %4")
            .arg(port.description(), port.manufacturer(),
                 port.serialNumber(), port.portName())
            .toLower();
    int score = 0;
    const QStringList positiveKeywords = {
        QStringLiteral("usb serial"), QStringLiteral("usb-serial"),
        QStringLiteral("usb 串行"), QStringLiteral("wch"),
        QStringLiteral("ch340"), QStringLiteral("ch341"),
        QStringLiteral("ftdi"), QStringLiteral("cp210"),
        QStringLiteral("silicon labs"), QStringLiteral("prolific"),
        QStringLiteral("uart")
    };
    const QStringList negativeKeywords = {
        QStringLiteral("active management technology"),
        QStringLiteral("amt - sol"), QStringLiteral("communications port"),
        QStringLiteral("通信端口"), QStringLiteral("bluetooth")
    };

    for (const QString &keyword : positiveKeywords) {
        if (text.contains(keyword))
            score += 100;
    }
    for (const QString &keyword : negativeKeywords) {
        if (text.contains(keyword))
            score -= 200;
    }
    if (port.hasVendorIdentifier())
        score += 80;
    if (port.hasProductIdentifier())
        score += 20;
    if (port.description().toLower().startsWith(QStringLiteral("usb")))
        score += 40;
    if (port.portName().compare(QStringLiteral("COM1"), Qt::CaseInsensitive) == 0 ||
        port.portName().compare(QStringLiteral("COM2"), Qt::CaseInsensitive) == 0) {
        score -= 20;
    }
    return score;
}

QString HexByte(uint8_t value)
{
    return QStringLiteral("0x%1")
            .arg(value, 2, 16, QLatin1Char('0'))
            .toUpper();
}

QString DeviceName(uint8_t addr)
{
    switch (addr) {
    case 0x01:
        return QStringLiteral("HIFU");
    case 0x02:
        return QStringLiteral("LIFU");
    case 0x03:
        return QStringLiteral("ROM");
    case 0x04:
        return QStringLiteral("SYSTEM");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

QString CommandName(uint8_t commandId)
{
    switch (commandId) {
    case 0x01:
        return QStringLiteral("OK");
    case 0x02:
        return QStringLiteral("RD_STATUS");
    case 0x03:
        return QStringLiteral("STATUS");
    case 0x04:
        return QStringLiteral("WR_HVOUT/HOST_CTRL");
    case 0x06:
        return QStringLiteral("WR_CHANNEL_SWITCH/CLINICAL_MODE");
    case 0x08:
        return QStringLiteral("WR_FREQUENCY");
    case 0x09:
        return QStringLiteral("FREQUENCY");
    case 0x0A:
        return QStringLiteral("WR_CHANNEL_DELAY");
    case 0x0C:
        return QStringLiteral("WR_PRI");
    case 0x0E:
        return QStringLiteral("WR_STARTSONIC");
    case 0x0F:
        return QStringLiteral("WR_STOPSONIC");
    case 0x10:
        return QStringLiteral("WR_TIMER");
    case 0x12:
        return QStringLiteral("WR_PD");
    case 0x14:
        return QStringLiteral("WR_ISPPA/WR_TRIGGER_MODE");
    case 0x18:
        return QStringLiteral("SN/EMIT_READY");
    case 0xFB:
        return QStringLiteral("FINISHED");
    case 0xFD:
        return QStringLiteral("ERROR");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

QString PacketSummary(const QByteArray &packet)
{
    if (packet.size() < 12)
        return QStringLiteral("packet too short, bytes=%1")
                .arg(packet.size());

    const uint8_t index = static_cast<uint8_t>(packet[2]);
    const uint8_t commandId = static_cast<uint8_t>(packet[3]);
    const uint8_t addr = static_cast<uint8_t>(packet[4]);
    const uint16_t len =
            (static_cast<uint8_t>(packet[5]) << 8) |
            static_cast<uint8_t>(packet[6]);
    const QByteArray payload = packet.mid(7, len);

    return QStringLiteral("packet idx=%1 cmd=%2(%3) addr=%4(%5) len=%6 data=%7")
            .arg(index)
            .arg(HexByte(commandId), CommandName(commandId))
            .arg(HexByte(addr), DeviceName(addr))
            .arg(len)
            .arg(QString(payload.toHex(' ')).toUpper());
}

bool IsStatusPollPacket(const QByteArray &packet)
{
    return packet.size() >= 5 &&
            static_cast<uint8_t>(packet[3]) == 0x02 &&
            static_cast<uint8_t>(packet[4]) == 0x04;
}

bool IsStatusFeedbackPacket(const QByteArray &packet)
{
    return packet.size() >= 5 &&
            static_cast<uint8_t>(packet[3]) == 0x03 &&
            static_cast<uint8_t>(packet[4]) == 0x04;
}

}

Q_GLOBAL_STATIC(SerialManager, serialManager);

SerialManager* SerialManager::GetInstance()
{
	return serialManager();
}

SerialManager::SerialManager()
{
    m_HeartTimer = new QTimer(this);
    connect(m_HeartTimer, &QTimer::timeout, this, &SerialManager::OnHeartTimeBeat);
    m_HeartTimer->setInterval(1000);
    m_HeartTimer->stop();

    m_SendTimeoutTimer = new QTimer(this);
    m_SendTimeoutTimer->setSingleShot(true);
    m_SendTimeoutTimer->setInterval(2000);
    connect(m_SendTimeoutTimer, &QTimer::timeout, this, &SerialManager::OnSendTimeout);

    m_InterCommandTimer = new QTimer(this);
    m_InterCommandTimer->setSingleShot(true);
    m_InterCommandTimer->setInterval(200);
    connect(m_InterCommandTimer, &QTimer::timeout,
            this, &SerialManager::OnInterCommandTimeout);

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
    if (m_HeartTimer && !m_HeartTimer->isActive())
        m_HeartTimer->start();
}

void SerialManager::HeartTimerStop()
{
    if (m_HeartTimer)
        m_HeartTimer->stop();
}

void SerialManager::SerialPortClose()
{
    HeartTimerStop();
    m_SendTimeoutTimer->stop();
    m_InterCommandTimer->stop();
    m_SendQueue.clear();
    m_Buffer.clear();
    m_IsSending = false;
    m_InterCommandDelaying = false;
    m_PendingCommandIsStatusPoll = false;
    m_PendingCommandId = 0;
    m_PendingAddr = 0;
    m_PendingIndex = 0;
    if (m_SerialPort && m_SerialPort->isOpen()) {
        m_SerialPort->close();
        emit writeLog(QStringLiteral("Serial port closed."));
    }
    emit serialPortStateChanged(false);
}

void SerialManager::SerialPortOpen()
{
    if (m_SerialPort && m_SerialPort->isOpen())
        return;
    InitSerialPort();
}

bool SerialManager::ParseSerialData(QByteArray &packet)
{
    static const QByteArray header = QByteArray::fromHex("5AA5");
    while (m_Buffer.size() >= 2)
    {
        const int pos = m_Buffer.indexOf(header);

        if (pos < 0)
        {
            const bool keepHeaderPrefix =
                    static_cast<uint8_t>(m_Buffer.back()) == 0x5A;
            m_Buffer = keepHeaderPrefix ? QByteArray(1, char(0x5A)) : QByteArray();
            return false;
        }

        if (pos > 0)
            m_Buffer.remove(0, pos);

        if (m_Buffer.size() < 7)
            return false;

        const uint16_t len =
                (static_cast<uint8_t>(m_Buffer[5]) << 8) |
                static_cast<uint8_t>(m_Buffer[6]);
        if (len > 1024)
        {
            m_Buffer.remove(0, 1);
            continue;
        }
        const int totalLen = 7 + len + 4 + 1;

        if (m_Buffer.size() < totalLen)
            return false;
        const QByteArray one = m_Buffer.left(totalLen);

        const uint8_t tail = static_cast<uint8_t>(one.at(totalLen - 1));
        if (tail != 0xDD)
        {
            m_Buffer.remove(0, 1);
            continue;
        }

        const uint32_t recvCrc =
                (static_cast<uint8_t>(one[7 + len]) << 24) |
                (static_cast<uint8_t>(one[7 + len + 1]) << 16) |
                (static_cast<uint8_t>(one[7 + len + 2]) << 8) |
                static_cast<uint8_t>(one[7 + len + 3]);
        const QByteArray crcRange = one.left(7 + len);

        if (Crc32(crcRange) != recvCrc)
        {
            emit writeLog(QStringLiteral("Receive CRC mismatch."));
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
    if (!m_SerialPort)
        return;

    m_Buffer.append(m_SerialPort->readAll());
    QByteArray packet;
    while (ParseSerialData(packet))
    {
        emit readSerialData(packet);
        const bool isStatusFeedback = IsStatusFeedbackPacket(packet);
        if (!isStatusFeedback)
            WriteQByteArrayLog(packet, true);
        const uint8_t commandId = static_cast<uint8_t>(packet[3]);
        if (commandId == 0xFB)
            emit writeLog(QStringLiteral("[MCU] emission finished feedback received"));

        const uint8_t packetIndex = static_cast<uint8_t>(packet[2]);
        if (m_IsSending && packetIndex == m_PendingIndex)
        {
            m_SendTimeoutTimer->stop();
            if (commandId == 0xFD) {
                emit writeLog(QStringLiteral("MCU rejected command, packet index: %1. Continue pending commands.")
                              .arg(packetIndex));
                emit commandRejected(m_PendingCommandId, m_PendingAddr);
                m_IsSending = false;
                m_PendingCommandIsStatusPoll = false;
                m_PendingCommandId = 0;
                m_PendingAddr = 0;
                m_PendingIndex = 0;
                m_InterCommandDelaying = true;
                m_InterCommandTimer->start();
                continue;
            }
            emit commandAccepted(m_PendingCommandId, m_PendingAddr);
            m_IsSending = false;
            m_PendingCommandIsStatusPoll = false;
            m_PendingCommandId = 0;
            m_PendingAddr = 0;
            m_PendingIndex = 0;
            m_InterCommandDelaying = true;
            m_InterCommandTimer->start();
        }
        else {
            if (!isStatusFeedback)
                emit writeLog(QStringLiteral("[MCU] asynchronous feedback accepted"));
        }
    }
}

void SerialManager::InitSerialPort()
{
    if (!m_SerialPort) {
        m_SerialPort = new QSerialPort(this);
        connect(m_SerialPort, &QSerialPort::readyRead,
                this, &SerialManager::OnSerialDataRead);
        connect(m_SerialPort, &QSerialPort::errorOccurred,
                this, &SerialManager::OnSerialError);
    }

    auto portList = QSerialPortInfo::availablePorts();
    if(portList.isEmpty())
    {
        qDebug("No available port detected.");
        emit writeLog("No available port detected.");
        emit serialPortStateChanged(false);
        return;
    }
    std::sort(portList.begin(), portList.end(),
              [](const QSerialPortInfo &left, const QSerialPortInfo &right) {
        const int leftScore = PortPriority(left);
        const int rightScore = PortPriority(right);
        if (leftScore != rightScore)
            return leftScore > rightScore;
        return left.portName() < right.portName();
    });

    const QSerialPortInfo selectedPort = portList.first();
    const QString portName = selectedPort.portName();
    if (portName.isEmpty()) {
        if (m_LastOpenError != QStringLiteral("No suitable serial port found.")) {
            qDebug("No suitable serial port found.");
            emit writeLog("No suitable serial port found.");
            m_LastOpenError = QStringLiteral("No suitable serial port found.");
        }
        emit serialPortStateChanged(false);
        return;
    }
    m_SerialPort->setPortName(portName);
    m_SerialPort->setBaudRate(QSerialPort::Baud115200);
    m_SerialPort->setDataBits(QSerialPort::Data8);
    m_SerialPort->setParity(QSerialPort::NoParity);
    m_SerialPort->setStopBits(QSerialPort::OneStop);
    m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);
    if (!m_SerialPort->open(QIODevice::ReadWrite))
	{
        const QString error = QStringLiteral("Open serial port %1 failed: %2")
                .arg(portName, m_SerialPort->errorString());
        if (m_LastOpenError != error) {
            qDebug() << error;
            emit writeLog(error);
            m_LastOpenError = error;
        }
        emit serialPortStateChanged(false);
		return;
    }
    m_LastOpenError.clear();
    m_SerialPort->clear(QSerialPort::AllDirections);
    m_HeartTimer->start();
    emit writeLog(QStringLiteral("Serial port opened: %1 (%2)")
                  .arg(portName, selectedPort.description()));
    emit serialPortStateChanged(true);
}

void SerialManager::Send(uint8_t cmd, uint8_t addr, uint16_t len,
                         QByteArray data, QString description)
{
    if (!m_SerialPort || !m_SerialPort->isOpen()) {
        emit writeLog(QStringLiteral("Send ignored: serial port is not open."));
        return;
    }
    if (len != static_cast<uint16_t>(data.size())) {
        emit writeLog(QStringLiteral("Packet length corrected from %1 to %2.")
                      .arg(len).arg(data.size()));
    }
    PendingCommand command;
    command.packet = PackPacket(
                cmd, addr, static_cast<uint16_t>(data.size()), data);
    command.description = description;
    m_SendQueue.enqueue(command);
    TrySendNext();
}

void SerialManager::Test(uint8_t cmd)
{
    Q_UNUSED(cmd);
    emit writeLog("Test");
}

void SerialManager::TrySendNext()
{
    if (m_IsSending)
        return;
    if (m_InterCommandDelaying)
        return;
    if (m_SendQueue.isEmpty())
        return;
    if (!m_SerialPort || !m_SerialPort->isOpen())
        return;

    const PendingCommand command = m_SendQueue.dequeue();
    const QByteArray packet = command.packet;
    m_IsSending = true;
    m_PendingIndex = static_cast<uint8_t>(packet[2]);
    m_PendingCommandId = static_cast<uint8_t>(packet[3]);
    m_PendingAddr = static_cast<uint8_t>(packet[4]);
    m_PendingCommandIsStatusPoll = IsStatusPollPacket(packet);
    if (!command.description.isEmpty())
        emit writeLog(QStringLiteral("[MCU] %1").arg(command.description));
    const qint64 bytesQueued = m_SerialPort->write(packet);
    if (bytesQueued != packet.size()) {
        emit writeLog(QStringLiteral("Serial write failed: %1")
                      .arg(m_SerialPort->errorString()));
        SerialPortClose();
        return;
    }
    if (!IsStatusPollPacket(packet))
        WriteQByteArrayLog(packet);
    m_SendTimeoutTimer->start();
}

void SerialManager::OnSendTimeout()
{
    emit writeLog(QStringLiteral("Serial response timeout, packet index: %1")
                  .arg(m_PendingIndex));
    if (m_PendingCommandIsStatusPoll) {
        emit writeLog(QStringLiteral("Status query timeout, serial port disconnected."));
        SerialPortClose();
        return;
    }
    emit commandRejected(m_PendingCommandId, m_PendingAddr);
    m_IsSending = false;
    m_PendingCommandIsStatusPoll = false;
    m_PendingCommandId = 0;
    m_PendingAddr = 0;
    m_PendingIndex = 0;
    m_InterCommandDelaying = true;
    m_InterCommandTimer->start();
    emit writeLog(QStringLiteral("Continue pending commands after timeout."));
}

void SerialManager::OnInterCommandTimeout()
{
    m_InterCommandDelaying = false;
    TrySendNext();
}

void SerialManager::OnSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError || !m_SerialPort)
        return;

    emit writeLog(QStringLiteral("Serial error: %1").arg(m_SerialPort->errorString()));
    if (error == QSerialPort::ResourceError)
        SerialPortClose();
}

void SerialManager::WriteQByteArrayLog(QByteArray data, bool isReceiveInfo)
{
    const QString direction = isReceiveInfo
            ? QStringLiteral("<--")
            : QStringLiteral("-->");
    emit writeLog(QStringLiteral("%1 %2")
                  .arg(direction, QString(data.toHex(' ')).toUpper()));
    emit writeLog(QStringLiteral("%1 %2")
                  .arg(direction, PacketSummary(data)));
}

QByteArray SerialManager::PackPacket(uint8_t cmd, uint8_t addr, uint16_t len, const QByteArray &data)
{
    QByteArray packet;

    packet.append((char)0x5A);
    packet.append((char)0xA5);
    packet.append(m_Index);
    m_Index = (m_Index + 1) & 0xFF;
    if (m_Index == 0)
        m_Index = 1;
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

uint32_t SerialManager::Crc32(const QByteArray &payload)
{
    static const uint32_t crc_table[256] = {
            0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
            0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
            0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
            0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
            0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
            0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
            0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
            0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
            0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
            0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
            0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
            0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
            0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
            0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
            0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
            0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
            0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
            0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
            0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
            0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
            0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
            0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
            0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
            0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
            0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
            0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
            0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
            0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
            0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
            0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
            0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
            0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
        };

        uint32_t crc = 0xFFFFFFFF;
        const char *data = payload.constData();
        const int len = payload.size();

        for (int i = 0; i < len; i++) {
            crc = (crc >> 8) ^ crc_table[(crc ^ static_cast<quint8>(data[i])) & 0xFF];
        }

        return crc ^ 0xFFFFFFFF;
}

void SerialManager::OnHeartTimeBeat()
{

    if (m_IsSending || !m_SendQueue.isEmpty())
        return;


    uint8_t deviceAddr = 0x04;
    uint8_t commandId = 0x02;
    uint16_t len = 4;
    QByteArray data(4, 0x00);
    Send(commandId, deviceAddr, len, data, QString());
}

