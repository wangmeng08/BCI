#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
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
    void TurnToPage(Page page);
    QThread *logThread = nullptr;

signals:
    void writeLog(LogType type, QString info);

private:
    QStackedWidget *m_Stack;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
