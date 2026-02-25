#ifndef CONSTVALUE_H
#define CONSTVALUE_H
#include "QObject"
#include <QMainWindow>

class ConstValue : public QObject
{
    Q_OBJECT
public:
    explicit ConstValue(QObject *parent = nullptr);
    static ConstValue* GetInstance();
    QStringList AnimalList;
    int DeleteLimit;
    int LoadInfo;
    QMainWindow *m_MainWindow = nullptr;;
};

#endif // CONSTVALUE_H
