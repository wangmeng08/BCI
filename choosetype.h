#ifndef CHOOSETYPE_H
#define CHOOSETYPE_H

#include <QWidget>
#include "enuminfo.h"

namespace Ui {
class ChooseType;
}

class ChooseType : public QWidget
{
    Q_OBJECT

public:
    explicit ChooseType(QWidget *parent = nullptr);
    ~ChooseType();

private:
    void OnClickHIFU();
    void OnClickLIFU();
    void OnClickLIFU4();

    void SetPage(Page page);
    Ui::ChooseType *ui;
};

#endif // CHOOSETYPE_H
