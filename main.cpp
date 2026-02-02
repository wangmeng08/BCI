#include "mainwindowhifu.h"
#include "mainwindowlifu.h"
#include "choosetype.h"
#include "datamanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int selectType = 0;
    ChooseType *dialog = new ChooseType(selectType);
    dialog->exec();
    if(selectType == 0){
        DataManager::GetInstance()->SetClinicalMode(ClinicalMode::HIFU);
        MainWindowHIFU w;
        w.show();
        return a.exec();
    }
    DataManager::GetInstance()->SetClinicalMode(ClinicalMode::LIFU);
    MainWindowLIFU w;
    w.show();
    return a.exec();
}
