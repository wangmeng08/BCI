#ifndef CONSTVALUE_H
#define CONSTVALUE_H
#include "QObject"

class ConstValue : public QObject
{
    Q_OBJECT
public:
    explicit ConstValue(QObject *parent = nullptr);
    static ConstValue* GetInstance();
    QStringList AnimalList;
};

#endif // CONSTVALUE_H
