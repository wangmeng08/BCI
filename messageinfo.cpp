#include "messageinfo.h"
#include "constvalue.h"
#include "ui_messageinfo.h"

MessageInfo::MessageInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageInfo)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowModality(Qt::ApplicationModal);
    connect(ui->btnAccept, &QPushButton::clicked, this, [=](){accept();});
    connect(ui->btnCancel, &QPushButton::clicked, this, [=](){reject();});
    connect(ui->btnConfirm, &QPushButton::clicked, this, [=](){accept();});
}

MessageInfo::~MessageInfo()
{
    delete ui;
}

void MessageInfo::SetInfo(QString info, QString title)
{
    ui->lblInfo->setText(info);
    ui->lblTitle->setText(title);
}

void MessageInfo::SetType(MessageType type)
{
    bool isQuestion = type == MessageType::QUESTION;
    ui->btnAccept->setVisible(isQuestion);
    ui->btnCancel->setVisible(isQuestion);
    ui->btnConfirm->setVisible(!isQuestion);
}

void MessageInfo::ShowInformation(QString info, QString title)
{
    ShowMessage(info, title, MessageType::TIPS);
}

void MessageInfo::ShowWarning(QString info, QString title)
{
    ShowMessage(info, title, MessageType::WARNING);
}

int MessageInfo::ShowMessage(QString info, QString title, MessageType type)
{
    MessageInfo *dialog = new MessageInfo(ConstValue::GetInstance()->m_MainWindow);
    dialog->SetInfo(info, title);
    dialog->SetType(type);
    dialog->resize(1024, 768);
    dialog->move(0, 0);
    return dialog->exec();
}

int MessageInfo::ShowQuestion(QString info, QString title)
{
    return ShowMessage(info, title, MessageType::QUESTION);
}
