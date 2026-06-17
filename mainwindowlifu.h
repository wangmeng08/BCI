#ifndef MAINWINDOWLIFU_H
#define MAINWINDOWLIFU_H
#include <QLineEdit>
#include <QMainWindow>
#include "basewindow.h"
#include "txitem.h"

namespace Ui {
class MainWindowLIFU;
}

class MainWindowLIFU : public BaseWindow
{
    Q_OBJECT

public:
    explicit MainWindowLIFU(QWidget *parent = nullptr);
    ~MainWindowLIFU();

    void InitProfileData() override;

protected:
    QLabel *GetConnectLabel() override;
    QLabel *GetEmitLabel() override;
    QLabel *GetStateIcon() override;
    void SendInitCommand() override;
    void SetTimerInfo() override;

private:
    void InitData();
    void InitEvent();

    void OnClickCancel();
    void OnClickEdit();
    void OnClickLoad();
    void OnClickSave();

    void OnCurrentProfileChange(QSharedPointer<ProfileLIFU> prev, QSharedPointer<ProfileLIFU> curr);
    void SetEditMode(bool isEdit);

    void UpdateBtnState() override;


    QVector<uint32_t> GetChannelDelay();
private:
    QVector<TXItem *> m_VectorItem;
    bool m_IsInEdit = false;
    Ui::MainWindowLIFU *ui;
};

#endif // MAINWINDOWLIFU_H
