#ifndef MAINWINDOWLIFU_H
#define MAINWINDOWLIFU_H

#include <QMainWindow>
#include "basewindow.h"

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
    Ui::MainWindowLIFU *ui;
};

#endif // MAINWINDOWLIFU_H
