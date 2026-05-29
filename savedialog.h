#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include <QDialog>

namespace Ui {
class SaveDialog;
}

class SaveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveDialog(int &saveType, QString &saveName, QWidget *parent = nullptr);
    ~SaveDialog();

private:
    void OnSaveType(int type);
    Ui::SaveDialog *ui;
    int *m_SaveType = nullptr;
    QString *m_SaveName = nullptr;
};

#endif // SAVEDIALOG_H
