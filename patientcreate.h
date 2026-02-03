#ifndef PATIENTCREATE_H
#define PATIENTCREATE_H

#include <QDialog>
#include <QPaintEvent>

#include "basedialog.h"

namespace Ui {
class PatientCreate;
}

class PatientCreate : public BaseDialog
{
    Q_OBJECT

public:
    explicit PatientCreate(QWidget *parent = nullptr);
    ~PatientCreate();

private:
    void OnCLickConfirm();
    Ui::PatientCreate *ui;
};

#endif // PATIENTCREATE_H
