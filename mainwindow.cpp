#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamanager.h"
#include "eventmanager.h"
#include "logmanager.h"
#include "mainwindowhifu.h"
#include "mainwindowlifu.h"
#include "patientmanager.h"
#include "qstackedwidget.h"

#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<LogType>("LogType");
    qRegisterMetaType<Page>("Page");
    qRegisterMetaType<TriggerMode>("TriggerMode");

    InitLogManager();
    QStackedWidget *stack = new QStackedWidget(this);

    MainWindowHIFU *hifu = new MainWindowHIFU;
    MainWindowLIFU *lifu = new MainWindowLIFU;
    PatientManager *patient = new PatientManager;
    stack->addWidget(patient);
    stack->addWidget(hifu); // index 0
    stack->addWidget(lifu); // index 1

    setCentralWidget(stack);
    DataManager::GetInstance()->SetClinicalMode(ClinicalMode::HIFU);
    stack->setCurrentIndex((int)Page::PatientManager);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitLogManager()
{
    LogManager *logMgr = LogManager::GetInstance();
    logThread = new QThread();
    logMgr->moveToThread(logThread);
    logMgr->timer->moveToThread(logThread);
    connect(logThread, &QThread::finished, logMgr, &QObject::deleteLater);
    logThread->start();
    connect(EventManager::GetInstance(), &EventManager::writeLog, logMgr, &LogManager::WriteLog);
}
