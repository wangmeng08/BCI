#ifndef PATIENTMANAGER_H
#define PATIENTMANAGER_H

#include <QWidget>

#include "datamanager.h"

namespace Ui {
class PatientManager;
}

class PatientManager : public QWidget
{
    Q_OBJECT

public:
    explicit PatientManager(QWidget *parent = nullptr);
    ~PatientManager();

private:
    void InitData();
    void InitEvent();
    void InitTable();
    void InitTableValue();
    void InitUI();

    void InsertPatientInfo(int row, QSharedPointer<Patient> patient);

    void OnClickNext();
    void OnClickNew();

    void OnDeletePatient(QSharedPointer<Patient> patient);
private:
    DataManager *dataManager;
    Ui::PatientManager *ui;
};

#endif // PATIENTMANAGER_H
