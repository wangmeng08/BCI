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
}

SerialManager::~SerialManager() {
	mSerialPort->close();
    delete mSerialPort;
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

void SerialManager::OnSerialDataRead()
{
}

//串口初始化
void SerialManager::InitSerialPort()
{
	// 串口标识
	mSerialPort = new QSerialPort;
	mSerialPort->setPortName("COM1");
	mSerialPort->setBaudRate(QSerialPort::Baud115200);
	mSerialPort->setDataBits(QSerialPort::Data8);
	mSerialPort->setParity(QSerialPort::NoParity);//设置奇偶校验
	mSerialPort->setStopBits(QSerialPort::OneStop);//设置停止位
	mSerialPort->setFlowControl(QSerialPort::NoFlowControl);//设置流控制
	connect(mSerialPort, &QSerialPort::readyRead, this, &SerialManager::OnSerialDataRead);
	if (!mSerialPort->open(QIODevice::ReadWrite))
	{
		std::cerr << "open failed" << std::endl;
		return;
    }
}

void SerialManager::OnHeartTimeBeat()
{

}

