#include "mainwindow.h"
#include "choosetype.h"
#include "datamanager.h"
#include <QFile>
#include <QFontDatabase>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(":/font/font.ttf");
    QString file = ":stylesheet.qss";

    QFile qss(file);
    if (qss.open(QFile::ReadOnly))
    {
        qDebug("open success");
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }
    else
    {
        qDebug("Open failed");
    }
    MainWindow w;
    w.show();
    return a.exec();
}
