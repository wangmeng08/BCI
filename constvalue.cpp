#include "constvalue.h"

Q_GLOBAL_STATIC(ConstValue, constValue);
ConstValue::ConstValue(QObject *parent)
{
    AnimalList.append(tr("Mouse"));
    AnimalList.append(tr("Rat"));
    AnimalList.append(tr("Rabbit"));
    AnimalList.append(tr("Large Animal"));
}

ConstValue *ConstValue::GetInstance()
{
    return constValue();
}
