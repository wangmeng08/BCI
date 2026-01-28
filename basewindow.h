#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include "enuminfo.h"
#include "datamanager.h"
#include "db.h"
class BaseWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit BaseWindow(QWidget *parent = nullptr);

protected:
    void InitDatabase();
    void InitLogManager();
    void InitSerialManager();

    void SetConnectState(ConnectState state);
    void SetEmitState(EmitState state);

    void WriteCommLog(QString info);
    void WriteDBLog(QString info);

    virtual QLabel *GetConnectLabel() = 0;
    virtual QLabel *GetEmitLabel() = 0;

    QStringList m_ConnectDesList = {tr("disconnected"), tr("connected")};
    QStringList m_ConnectQss = {"color:rgba(128,128,128,128)", "color:rgba(0,255,0,128)"};

    QStringList m_EmitDesList = {tr("Idle"), tr("On"), tr("Error")};
    QStringList m_EmitQss = {"color:rgba(128,128,128,128)", "color:rgba(0,255,0,128)", "color:rgba(128,128,128,128)"};

    QThread *serialPortThread = nullptr;
    QThread *logThread = nullptr;

    DB *m_DB = nullptr;

    ConnectState m_ConnectSate = ConnectState::DISCONNECT;
    EmitState m_State = EmitState::IDLE;
    DataManager *m_DataManager = nullptr;
signals:
    void heartTimerStart();
    void heartTimerStop();
    void serialPortClose();
    void serialPortOpen();
    void writeLog(LogType type, QString info);
};

#endif // BASEWINDOW_H
