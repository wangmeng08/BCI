#include "reportitem.h"
#include "ui_reportitem.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

ReportItem::ReportItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReportItem)
{
    ui->setupUi(this);
    connect(ui->btnOpen, &QPushButton::clicked, this, &ReportItem::OnClickOpen);
}

ReportItem::~ReportItem()
{
    delete ui;
}

void ReportItem::SetInfo(QString path, QString date)
{
    m_FilePath = path;
    QFileInfo fileInfo(path);
    QString fileName = fileInfo.completeBaseName();
    ui->lblName->setText(QString("Report Name: %1").arg(fileName));
    ui->lblDate->setText(QString("Date: %1").arg(date));
}

void ReportItem::OnClickOpen()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_FilePath));
}
