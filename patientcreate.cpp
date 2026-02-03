#include "patientcreate.h"
#include "constvalue.h"
#include "datamanager.h"
#include "messageinfo.h"
#include "ui_patientcreate.h"

#include <QPainter>

PatientCreate::PatientCreate(QWidget *parent) :
    BaseDialog(parent),
    ui(new Ui::PatientCreate)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    ui->cmbKind->addItems(ConstValue::GetInstance()->AnimalList);
    connect(ui->btnCancel, &QPushButton::clicked, this, &PatientCreate::reject);
    connect(ui->btnConfirm, &QPushButton::clicked, this, &PatientCreate::OnCLickConfirm);
}

PatientCreate::~PatientCreate()
{
    delete ui;
}

void PatientCreate::OnCLickConfirm()
{
    QString name = ui->lblName->text().trimmed();
    QString illness = ui->lblIllness->text().trimmed();
    QString age = ui->lblAge->text().trimmed();
    QString weight = ui->lblWeight->text().trimmed();
    if(name == "" || illness == "" || age == "" || weight == "")
    {
        MessageInfo::ShowInformation(tr("All fields must be filled."));
        return;
    }
    bool res = DataManager::GetInstance()->CreatePatient(name, illness, age, weight, ui->cmbKind->currentIndex());
    if(res)
        accept();
}
