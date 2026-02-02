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
    QSerialPort *mSerialPort;
    QTimer *heartTimer;


public slots:
    void HeartTimerStart();
    void HeartTimerStop();
    void SerialPortClose();
    void SerialPortOpen();

private:
    QList<QByteArray> serialDataList;
    void InitSerialPort();

protected Q_SLOTS:
    void OnHeartTimeBeat();
    void OnSerialDataRead();

Q_SIGNALS:
	void writeLog(QString info);

public slots:
};
