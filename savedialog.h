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
    explicit SaveDialog(int &saveType, QWidget *parent = nullptr);
    ~SaveDialog();

private:
    void OnSaveType(int type);
    Ui::SaveDialog *ui;
    int *m_SaveType = nullptr;
};

#endif // SAVEDIALOG_H
