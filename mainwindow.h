#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "enuminfo.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void InitLogManager();

    QThread *logThread = nullptr;

signals:
    void writeLog(LogType type, QString info);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
