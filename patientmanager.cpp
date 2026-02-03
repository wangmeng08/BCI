#include "choosetype.h"
#include "constvalue.h"
#include "eventmanager.h"
#include "messageinfo.h"
#include "patientcreate.h"
#include "patientmanager.h"
#include "ui_patientmanager.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QTableWidgetItem>

PatientManager::PatientManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientManager)
{
    ui->setupUi(this);
    InitUI();
    InitData();
    InitTable();
    InitTableValue();
    InitEvent();
}

PatientManager::~PatientManager()
{
    delete ui;
}

void PatientManager::InitData()
{
    dataManager = DataManager::GetInstance();
}

void PatientManager::InitEvent()
{
    connect(ui->btnNext, &QPushButton::clicked, this, &PatientManager::OnClickNext);
    connect(ui->btnNew, &QPushButton::clicked, this, &PatientManager::OnClickNew);
}

void PatientManager::InitTable()
{
    auto header = ui->tableInfo->horizontalHeader();
    ui->tableInfo->verticalHeader()->hide();

    const QStringList list = {"id", "Name", "Kind", "Illness", "Age", "Weight", "Operate"};
    ui->tableInfo->setColumnCount(list.count());
    ui->tableInfo->setHorizontalHeaderLabels(list);

    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::Stretch);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
    header->setSectionResizeMode(5, QHeaderView::Stretch);
    header->setSectionResizeMode(6, QHeaderView::Stretch);
    ui->tableInfo->horizontalHeader()->setFixedHeight(50);
    ui->tableInfo->verticalHeader()->setDefaultSectionSize(50);
    ui->tableInfo->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableInfo->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableInfo->setColumnHidden(0, true);

}

void PatientManager::InitTableValue()
{
    ui->tableInfo->setRowCount(0);
    int num = dataManager->m_PatientList.count();
    for(int i=0; i<num; i++)
    {
        InsertPatientInfo(i, dataManager->m_PatientList[i]);
    }
}


void PatientManager::InitUI()
{

}

void PatientManager::InsertPatientInfo(int row, QSharedPointer<Patient> patient)
{
    ui->tableInfo->insertRow(row);
    QStringList list;
    list.append(QString::number(patient->index));
    list.append(patient->name);
    list.append(ConstValue::GetInstance()->AnimalList[patient->animalType]);
    list.append(patient->illness);
    list.append(QString::number(patient->age));
    list.append(QString::number(patient->weight));
    for(int i=0; i<list.size(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(list[i]);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableInfo->setItem(row, i, item);
    }
    QPushButton *btn = new QPushButton("Delete");
    btn->setFixedSize(120, 48);
    connect(btn, &QPushButton::clicked, this, [=](){
        OnDeletePatient(patient);
    });
    QWidget *widget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(btn, 0, Qt::AlignCenter);
    widget->setLayout(layout);
    ui->tableInfo->setCellWidget(row, list.size(), widget);
}

void PatientManager::OnClickNext()
{
    int index = ui->tableInfo->currentRow();
    if(index < 0 || index >= dataManager->m_PatientList.size())
    {
        MessageInfo::ShowInformation(tr("Please select a patient first"));
        return;
    }
    dataManager->m_CurrentPatient = dataManager->m_PatientList[index];
    int selectType = 0;
    ChooseType *dialog = new ChooseType(selectType);
    dialog->exec();
    if(selectType == 0)
        return;
    Page page = (Page)selectType;
    ClinicalMode mode = (ClinicalMode)(selectType);
    dataManager->SetClinicalMode(mode);
    emit EventManager::GetInstance()->turnToPage(page);
}

void PatientManager::OnClickNew()
{
    auto dialog = new PatientCreate(this);
    auto size2 = this->size();
    dialog->resize(size2);
    dialog->move(0, 0);
    auto res = dialog->exec();
    delete dialog;
    if(res == 1)
    {
        int num = dataManager->m_PatientList.size() - 1;
        InsertPatientInfo(num, dataManager->m_PatientList[num]);
    }
}

void PatientManager::OnDeletePatient(QSharedPointer<Patient> patient)
{
    QString info = QString("Delete patients whose name is %1").arg(patient->name);
    auto res = MessageInfo::ShowQuestion(info);
    if(res == 0) return;
    auto success = dataManager->DeletePatient(patient);
    if(!success) return;
    int rowNum = ui->tableInfo->rowCount();
    for(int j=0; j<rowNum; j++)
    {
        int tmpIndex = ui->tableInfo->item(j, 0)->text().toInt();
        if(tmpIndex == patient->index)
        {
            ui->tableInfo->removeRow(j);
            break;
        }
    }

}
