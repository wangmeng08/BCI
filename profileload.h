#ifndef PROFILELOAD_H
#define PROFILELOAD_H

#include <QDialog>

namespace Ui {
class ProfileLoad;
}

class ProfileLoad : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileLoad(int &loadIndex,  QWidget *parent = nullptr);
    ~ProfileLoad();

private:
    void OnClickCancle();
    void OnClickLoad();
    Ui::ProfileLoad *ui;
    int *m_LoadIndex = nullptr;
    int m_CurrentIndex = 0;
};

#endif // PROFILELOAD_H
