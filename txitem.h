#ifndef TXITEM_H
#define TXITEM_H

#include <QWidget>

namespace Ui {
class TXItem;
}

class TXItem : public QWidget
{
    Q_OBJECT

public:
    explicit TXItem(int index, QWidget *parent = nullptr);
    ~TXItem();

    void SetInfo(int num);
    void SetItemEnable(bool state);

    int GetInfo();
private:
    Ui::TXItem *ui;
};

#endif // TXITEM_H
