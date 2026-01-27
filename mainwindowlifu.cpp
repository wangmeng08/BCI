#include "mainwindowlifu.h"
#include "ui_mainwindowlifu.h"

MainWindowLIFU::MainWindowLIFU(QWidget *parent) :
    BaseWindow(parent),
    ui(new Ui::MainWindowLIFU)
{
    ui->setupUi(this);
}

MainWindowLIFU::~MainWindowLIFU()
{
    delete ui;
}

QLabel *MainWindowLIFU::GetConnectLabel()
{
    return nullptr;
}

QLabel *MainWindowLIFU::GetEmitLabel()
{
    return nullptr;
}
