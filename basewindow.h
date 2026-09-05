#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QLabel>
#include <QLineEdit>
#include <QThread>
#include <QTimer>
#include <QWidget>
#include "enuminfo.h"
#include "datamanager.h"
#include "db.h"
class BaseWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWindow(QWidget *parent = nullptr);
    virtual void InitProfileData() = 0;
protected:
    uint8_t GetDeviceAddr();
    uint32_t GetValueFromQByteArray(QByteArray data, int startIndex, int len);

    void EmitTimerJump();
    void EmitTimerStart();
    void EmitTimerStop();

    void InitDatabase();
    void InitSerialManager();

    void OnClickOff();
    void OnClickOn();

    void ReadDataHIFU(QByteArray data);
    void ReadDataLIFU(QByteArray data);
    void ReadDataSystem(QByteArray data);
    void ReadSerialData(QByteArray data);

    void OnCommandAccepted(uint8_t commandId, uint8_t addr);
    void OnCommandRejected(uint8_t commandId, uint8_t addr);
    void Send(uint8_t cmd, uint8_t addr, uint16_t len, QByteArray data,
              const QString &description = QString());
    void SendCommandData4(uint8_t commandId, uint32_t value,
                          const QString &description);

    void SendCommandSetEmitTime(uint32_t value);
    void SendCommandSetFrequency(uint32_t value);
    void SendCommandSetHvout(double value);
    void SendCommandSetIsppa(double value);
    void SendCommandSetPD(uint32_t value);
    void SendCommandSetPri(uint32_t value);

    void SendCommandSetChannelSwitch(int channelCount);
    void SendCommandSetChannelDelay(const QVector<uint32_t>& delays);

    void SendCommandSystemEmit();
    void SendCommandSystemStop();
    void SendCommandSystemEmitReady();
    void SendCommandSystemHostConnectStatus(HostControlMode mode);
    void SendCommandSystemHostCheckStatus();
    void SendCommandSystemHostCheckSN();
    void SendCommandSystemModel();
    void SendCommandSystemTriggerModel();

    void SetConnectState(ConnectState state);
    bool IsSerialOpen() const;
    bool IsSerialAvailable() const;
    void SyncConnectStateFromSerial();
    void SetEmitState(EmitState state);
    void SetLineEditState(QLineEdit* edit, bool enable);
    void SetUnderline(QLineEdit* edit, bool enable);

    void WriteCommLog(QString info);

    virtual QLabel *GetConnectLabel() = 0;
    virtual QLabel *GetEmitLabel() = 0;
    virtual QLabel *GetStateIcon() = 0;
    virtual void SendInitCommand() = 0;
    virtual void SetTimerInfo() = 0;
    virtual void UpdateBtnState() = 0;
    virtual void OnSonicStarted(uint8_t addr);
    virtual void OnSonicStopped();

    QByteArray FromUint32(uint32_t value);

    int m_CurrentTime = 0;
    const int m_timerIntervalMs = 50;

    QStringList m_ConnectDesList = {tr("disconnected"), tr("standby"), tr("overheat"), tr("overcurrent"), tr("normal output")};
    QStringList m_ConnectIconQss = {"border-radius: 15px; background-color:#C0CBDF;", "border-radius: 15px; background-color:#44C063;"};
    QStringList m_ConnectQss = {"color:rgba(128,128,128,128)", "color:rgba(0,255,0,128)", "color:rgba(0,255,0,128)", "color:rgba(0,255,0,128)", "color:rgba(0,255,0,128)"};

    QStringList m_EmitDesList = {tr("Idle"), tr("On"), tr("Error")};
    QStringList m_EmitQss = {"color:rgba(128,128,128,128)", "color:rgba(0,255,0,128)", "color:rgba(128,128,128,128)"};

    QTimer *m_Timer = nullptr;
    QThread *serialPortThread = nullptr;

    DB *m_DB = nullptr;

    ConnectState m_ConnectState = ConnectState::DISCONNECT;
    EmitState m_State = EmitState::IDLE;
    bool m_EmitStartPending = false;
    DataManager *m_DataManager = nullptr;
signals:
    void heartTimerStart();
    void heartTimerStop();

    void send(uint8_t cmd, uint8_t addr, uint16_t len, QByteArray data,
              QString description);

    void serialPortClose();
    void serialPortOpen();

    void test(uint8_t cmd);
};

#endif // BASEWINDOW_H
