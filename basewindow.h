#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QLabel>
#include <QLineEdit>
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
    void EmitTimerJump();
    void EmitTimerStart();
    void EmitTimerStop();

    void InitDatabase();
    void InitSerialManager();

    void OnClickOff();
    void OnClickOn();

    void SetConnectState(ConnectState state);
    void SetEmitState(EmitState state);
    void SetLineEditState(QLineEdit* edit, bool enable);
    void SetUnderline(QLineEdit* edit, bool enable);

    void WriteCommLog(QString info);

    virtual QLabel *GetConnectLabel() = 0;
    virtual QLabel *GetEmitLabel() = 0;
    virtual QLabel *GetStateIcon() = 0;
    virtual void SetTimerInfo() = 0;
    virtual void UpdateBtnState() = 0;

    int m_CurrentTime = 0;

    QStringList m_ConnectDesList = {tr("disconnected"), tr("connected")};
    QStringList m_ConnectIconQss = {"border-radius: 15px; background-color:#C0CBDF;", "border-radius: 15px; background-color:#44C063;"};
    QStringList m_ConnectQss = {"color:rgba(128,128,128,128)", "color:rgba(0,255,0,128)"};

    QStringList m_EmitDesList = {tr("Idle"), tr("On"), tr("Error")};
    QStringList m_EmitQss = {"color:rgba(128,128,128,128)", "color:rgba(0,255,0,128)", "color:rgba(128,128,128,128)"};

    QTimer *m_Timer = nullptr;
    QThread *serialPortThread = nullptr;

    DB *m_DB = nullptr;

    ConnectState m_ConnectSate = ConnectState::DISCONNECT;
    EmitState m_State = EmitState::IDLE;
    DataManager *m_DataManager = nullptr;
signals:
    void heartTimerStart();
    void heartTimerStop();
    void serialPortClose();
    void serialPortOpen();
};

#endif // BASEWINDOW_H
