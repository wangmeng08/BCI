#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include "basewindow.h"
#include "datamanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public BaseWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    DataManager *m_DataManager = nullptr;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
