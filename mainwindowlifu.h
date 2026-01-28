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


protected:
    QLabel *GetConnectLabel() override;
    QLabel *GetEmitLabel() override;

private:
    void InitData();
    void InitEvent();

    void OnClickCancel();
    void OnClickEdit();
    void OnClickLoad();
    void OnClickOff();
    void OnClickOn();
    void OnClickSave();

    void SetEditMode(bool isEdit);

    void UpdateBtnState();

private:
    QVector<TXItem *> m_VectorItem;
    bool m_IsInEdit = false;
    Ui::MainWindowLIFU *ui;
};

#endif // MAINWINDOWLIFU_H
