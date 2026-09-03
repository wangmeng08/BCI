#pragma once

#include <qobject.h>
#include <QThread>
#include <QSerialPort> 
#include <QSerialPortInfo>
#include <QGlobalStatic>
#include <QDebug>
#include <QTimer>
#include <QQueue>

class SerialManager: public QObject
{
	Q_OBJECT
public:
	static SerialManager* GetInstance();

public:
	SerialManager();
    ~SerialManager();
    QSerialPort *m_SerialPort = nullptr;
    QTimer *m_HeartTimer = nullptr;
    QTimer *m_SendTimeoutTimer = nullptr;
    QTimer *m_InterCommandTimer = nullptr;


public slots:
    void HeartTimerStart();
    void HeartTimerStop();
    void SerialPortClose();
    void SerialPortOpen();
    void Send(uint8_t cmd, uint8_t addr, uint16_t len, QByteArray data,
              QString description);

    void Test(uint8_t cmd);

private:
    uint8_t m_Index = 1;
    QByteArray m_Buffer;

    struct PendingCommand {
        QByteArray packet;
        QString description;
    };
    QQueue<PendingCommand> m_SendQueue;
    bool m_IsSending = false;
    bool m_InterCommandDelaying = false;
    bool m_PendingCommandIsStatusPoll = false;
    uint8_t m_PendingCommandId = 0;
    uint8_t m_PendingAddr = 0;
    uint8_t m_PendingIndex = 0;
    QString m_LastOpenError;

    bool ParseSerialData(QByteArray& packet);

    void InitSerialPort();
    void TrySendNext();
    void WriteQByteArrayLog(QByteArray data, bool isReceiveInfo = false);

    QByteArray PackPacket(uint8_t cmd, uint8_t addr, uint16_t len, const QByteArray& data);

    uint32_t Crc32(const QByteArray& data);

protected slots:
    void OnHeartTimeBeat();
    void OnSerialDataRead();
    void OnSendTimeout();
    void OnInterCommandTimeout();
    void OnSerialError(QSerialPort::SerialPortError error);

Q_SIGNALS:
    void readSerialData(QByteArray data);
	void writeLog(QString info);
    void serialPortStateChanged(bool isOpen);
    void commandAccepted(uint8_t commandId, uint8_t addr);
    void commandRejected(uint8_t commandId, uint8_t addr);

public slots:
};
