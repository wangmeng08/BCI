#ifndef CHOOSETYPE_H
#define CHOOSETYPE_H

#include <QDialog>

namespace Ui {
class ChooseType;
}

class ChooseType : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseType(int &selectType, QWidget *parent = nullptr);
    ~ChooseType();

private:
    void OnClickHIFU();
    void OnClickLIFU();
    Ui::ChooseType *ui;
    int *m_SelectType;
};

#endif // CHOOSETYPE_H
