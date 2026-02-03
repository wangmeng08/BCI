#ifndef MAINWINDOWHIFU_H
#define MAINWINDOWHIFU_H

#include <QMainWindow>
#include <QLineEdit>
#include "basewindow.h"
#include "datamanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowHIFU; }
QT_END_NAMESPACE

class MainWindowHIFU : public BaseWindow
{
    Q_OBJECT

public:
    MainWindowHIFU(QWidget *parent = nullptr);
    ~MainWindowHIFU();

    void InitProfileData() override;
protected:
    QLabel *GetConnectLabel() override;
    QLabel *GetEmitLabel() override;

private:
    void InitData();
    void InitEvent();

    void OnClickAdvance();
    void OnClickCancel();
    void OnClickEdit();
    void OnClickLoad();
    void OnClickLocal();
    void OnClickOff();
    void OnClickOn();
    void OnClickOption();
    void OnClickSave();

    void OnModeAdvanceRFChange();
    void OnModePowerLimitChange();
    void OnModeTriggerChange();

    void SetAdvanceBtnState();
    void SetEditMode(bool isEdit);

    void UpdateBtnState();

private:
    QVector<QLineEdit *> m_VectorListDelay;
    QVector<QLineEdit *> m_VectorListFreq;
    QVector<QLineEdit *> m_VectorListHCD;
    bool m_IsInAdvance = false;
    bool m_IsInEdit = false;
    Ui::MainWindowHIFU *ui;
};
#endif // MAINWINDOWHIFU_H
