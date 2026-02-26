#ifndef REPORTITEM_H
#define REPORTITEM_H

#include <QWidget>

namespace Ui {
class ReportItem;
}

class ReportItem : public QWidget
{
    Q_OBJECT

public:
    explicit ReportItem(QWidget *parent = nullptr);
    ~ReportItem();

    void SetInfo(QString path, QString date);
private:
    void OnClickOpen();

    QString m_FilePath;
    Ui::ReportItem *ui;
};

#endif // REPORTITEM_H
