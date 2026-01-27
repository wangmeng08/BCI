#ifndef MESSAGEINFO_H
#define MESSAGEINFO_H

#include <QDialog>
#include "enuminfo.h"
namespace Ui {
class MessageInfo;
}

class MessageInfo : public QDialog
{
    Q_OBJECT

public:
    explicit MessageInfo(QWidget *parent = nullptr);
    ~MessageInfo();
    void SetInfo(QString info, QString title);
    void SetType(MessageType type);

    void static ShowInformation(QString info, QString title="Tips");
    void static ShowWarning(QString info, QString title="Warning");
    int static ShowMessage(QString info, QString title, MessageType type);
    int static ShowQuestion(QString info, QString title="Tips");
private:
    Ui::MessageInfo *ui;
};

#endif // MESSAGEINFO_H
