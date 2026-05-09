#pragma once

#include <qobject.h>
#include <QThread>
#include <QSerialPort> 
#include <QSerialPortInfo>
#include <QGlobalStatic>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QFile>
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
    QTimer *m_PostSendDelayTimer = nullptr;


public slots:
    void HeartTimerStart();
    void HeartTimerStop();
    void SerialPortClose();
    void SerialPortOpen();
    void Send(uint8_t cmd, uint8_t addr, uint16_t len, QByteArray data);

    void Test(uint8_t cmd);

private:
    uint8_t m_Index = 0;
    QByteArray m_Buffer;
    QList<QByteArray> serialDataList;

    QQueue<QByteArray> m_SendQueue;
    bool m_IsSending = false;
    bool m_PostSendDelaying = false;

    bool ParseSerialData(QByteArray& packet);
    bool ShouldDelaySend(QByteArray& packet);

    void InitSerialPort();
    void TrySendNext();
    void WriteQByteArrayLog(QByteArray data, bool isReceiveInfo = false);

    QByteArray PackPacket(uint8_t cmd, uint8_t addr, uint16_t len, const QByteArray& data);

    uint32_t Crc32(const QByteArray& data);

protected slots:
    void OnHeartTimeBeat();
    void OnSerialDataRead();
    void OnSendTimeout();
    void OnPostSendDelayTimeout();

Q_SIGNALS:
    void readSerialData(QByteArray data);
	void writeLog(QString info);

public slots:
};
