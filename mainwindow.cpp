#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "choosetype.h"
#include "constvalue.h"
#include "datamanager.h"
#include "eventmanager.h"
#include "logmanager.h"
#include "mainwindowhifu.h"
#include "mainwindowlifu.h"
#include "mainwindowlifu4.h"
#include "patientmanager.h"

#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    qRegisterMetaType<LogType>("LogType");
    qRegisterMetaType<Page>("Page");
    qRegisterMetaType<TriggerMode>("TriggerMode");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");

    InitLogManager();
    m_Stack = new QStackedWidget(this);
    ChooseType *choose = new ChooseType;
    PatientManager *patient = new PatientManager;
    auto addPage = [&](Page page, QWidget* w){
        m_Stack->insertWidget(static_cast<int>(page), w);
    };
    addPage(Page::Choose, choose);
    addPage(Page::PatientManager, patient);

    ConstValue::GetInstance()->m_MainWindow = this;

    setCentralWidget(m_Stack);
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
    if(page == Page::PatientManager || page == Page::Choose)
    {
        m_Stack->setCurrentIndex((int)page);
    }
    else{
        BaseWindow *window = nullptr;

        if(page == Page::HIFU)
            window = new MainWindowHIFU;
        else if(page == Page::LIFU)
            window = new MainWindowLIFU;
        else
            window = new MainWindowLIFU4;
        m_Stack->addWidget(window);
        m_Stack->setCurrentWidget(window);
        window->InitProfileData();
    }
}
