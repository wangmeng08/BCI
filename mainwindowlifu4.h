#ifndef MAINWINDOWLIFU4_H
#define MAINWINDOWLIFU4_H
#include <QLineEdit>
#include <QMainWindow>
#include "basewindow.h"
#include "txitem.h"

namespace Ui {
class MainWindowLIFU4;
}

class MainWindowLIFU4 : public BaseWindow
{
    Q_OBJECT

public:
    explicit MainWindowLIFU4(QWidget *parent = nullptr);
    ~MainWindowLIFU4();

    void InitProfileData() override;

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
    Ui::MainWindowLIFU4 *ui;
};

#endif // MAINWINDOWLIFU4_H
