#include "profileload.h"
#include "constvalue.h"
#include "ui_profileload.h"
#include "datamanager.h"
#include "messageinfo.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>

ProfileLoad::ProfileLoad(int &loadIndex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileLoad)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModality::ApplicationModal);
    ui->btnLoadFile->hide();
    if(DataManager::GetInstance()->GetClinicalMode() == ClinicalMode::HIFU)
    {
        for(int i=0; i<DataManager::GetInstance()->m_ProfileList.size(); i++)
        {
            auto profile = DataManager::GetInstance()->m_ProfileList[i];
            ui->cmbProfile->addItem(profile->profileName);
            if(profile == DataManager::GetInstance()->m_CurrentProfile)
            {
                m_CurrentIndex = i;
            }
        }
        ui->cmbProfile->setCurrentIndex(m_CurrentIndex);
    }
    else if(DataManager::GetInstance()->GetClinicalMode() == ClinicalMode::LIFU128)
    {
        for(int i=0; i<DataManager::GetInstance()->m_ProfileListLIFU.size(); i++)
        {
            auto profile = DataManager::GetInstance()->m_ProfileListLIFU[i];
            ui->cmbProfile->addItem(profile->profileName);
            if(profile == DataManager::GetInstance()->m_CurrentProfileLIFU)
            {
                m_CurrentIndex = i;
            }
        }
        ui->cmbProfile->setCurrentIndex(m_CurrentIndex);
        ui->btnLoadFile->show();
    }
    else if(DataManager::GetInstance()->GetClinicalMode() == ClinicalMode::LIFU4)
    {
        for(int i=0; i<DataManager::GetInstance()->m_ProfileListLIFU4.size(); i++)
        {
            auto profile = DataManager::GetInstance()->m_ProfileListLIFU4[i];
            ui->cmbProfile->addItem(profile->profileName);
            if(profile == DataManager::GetInstance()->m_CurrentProfileLIFU4)
            {
                m_CurrentIndex = i;
            }
        }
        ui->cmbProfile->setCurrentIndex(m_CurrentIndex);
    }
    m_LoadIndex = &loadIndex;
    connect(ui->btnCancel, &QPushButton::clicked, this, &ProfileLoad::OnClickCancle);
    connect(ui->btnDelete, &QPushButton::clicked, this, &ProfileLoad::OnClickDelete);
    connect(ui->btnLoad, &QPushButton::clicked, this, &ProfileLoad::OnClickLoad);
    connect(ui->btnLoadFile, &QPushButton::clicked, this, &ProfileLoad::OnClickLoadInfo);
}

ProfileLoad::~ProfileLoad()
{
    delete ui;
}

void ProfileLoad::OnClickCancle()
{
    *m_LoadIndex = -1;
    reject();
}

void ProfileLoad::OnClickDelete()
{
    if(0 == ui->cmbProfile->currentIndex())
    {
        MessageInfo::ShowInformation("Default profile cannot be deleted.");
        return;
    }
    QString info = QString("Delete profile whose name is %1").arg(ui->cmbProfile->currentText());
    auto res = MessageInfo::ShowQuestion(info);
    if(res == 0) return;
    *m_LoadIndex = ui->cmbProfile->currentIndex() + ConstValue::GetInstance()->DeleteLimit;
    accept();
}

void ProfileLoad::OnClickLoad()
{
    if(m_CurrentIndex == ui->cmbProfile->currentIndex())
    {
        OnClickCancle();
        return;
    }
    *m_LoadIndex = ui->cmbProfile->currentIndex();
    accept();
}

void ProfileLoad::OnClickLoadInfo()
{
    QString filePath = QFileDialog::getOpenFileName(
            this,
            "Select Data File",
            QDir::homePath(),
            "Text Files (*.txt);;All Files (*)"
        );
    if (filePath.isEmpty())
        return;
    DataManager::GetInstance()->m_TempLIFUValue = ReadFileToVector(filePath);
    if(DataManager::GetInstance()->m_TempLIFUValue.size() != 128)
    {
        MessageInfo::ShowInformation("Data file format error!");
        return;
    }
    *m_LoadIndex = ConstValue::GetInstance()->LoadInfo;
    accept();
}

QVector<int> ProfileLoad::ReadFileToVector(const QString &filePath)
{
    QVector<int> values;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return values;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QStringList list = line.split(",", Qt::SkipEmptyParts);

        for (const QString& str : list) {
            bool ok;
            int num = str.toInt(&ok);
            if (ok) {
                values.append(num);
            }
        }
    }

    file.close();
    return values;
}
