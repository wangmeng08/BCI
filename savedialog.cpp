#include "savedialog.h"
#include "messageinfo.h"
#include "ui_savedialog.h"

SaveDialog::SaveDialog(int &saveType, QString &saveName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    m_SaveType = &saveType;
    m_SaveName = &saveName;
    connect(ui->btnSave, &QPushButton::clicked, this, [=]() {OnSaveType(0);});
    connect(ui->btnSaveNew, &QPushButton::clicked, this, [=]() {OnSaveType(1);});
    connect(ui->btnConfirm, &QPushButton::clicked, this, [=]() {OnSaveType(2);});
    connect(ui->btnCancel, &QPushButton::clicked, this, [=]() {OnSaveType(3);});
}

SaveDialog::~SaveDialog()
{
    delete ui;
}

void SaveDialog::OnSaveType(int type)
{
    *m_SaveName = ui->lineEdit->text();
    if(type == 1 && *m_SaveName == "")
    {
        MessageInfo::ShowInformation(tr("The new profile name cannot be empty."));
        return;
    }
    *m_SaveType = type;
    accept();
}
