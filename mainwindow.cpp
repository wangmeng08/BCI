#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamanager.h"
#include "eventmanager.h"
#include "logmanager.h"
#include "mainwindowhifu.h"
#include "mainwindowlifu.h"
#include "patientmanager.h"

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
    m_Stack = new QStackedWidget(this);

    MainWindowHIFU *hifu = new MainWindowHIFU;
    MainWindowLIFU *lifu = new MainWindowLIFU;
    PatientManager *patient = new PatientManager;
    auto addPage = [&](Page page, QWidget* w){
        m_Stack->insertWidget(static_cast<int>(page), w);
    };
    addPage(Page::PatientManager, patient);
    addPage(Page::HIFU, hifu);
    addPage(Page::LIFU, lifu);

    setCentralWidget(m_Stack);
    DataManager::GetInstance()->SetClinicalMode(ClinicalMode::HIFU);
    TurnToPage(Page::PatientManager);
    connect(EventManager::GetInstance(), &EventManager::turnToPage, this, &MainWindow::TurnToPage);
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

void MainWindow::TurnToPage(Page page)
{
    m_Stack->setCurrentIndex((int)page);
}
