#include "savedialog.h"
#include "ui_savedialog.h"

SaveDialog::SaveDialog(int &saveType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    m_SaveType = &saveType;
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
    *m_SaveType = type;
    accept();
}
