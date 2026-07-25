#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MyThread.h"

class QPushButton;
class QProgressBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStart();
    void onProgress(int value);
    void onDone();

private:
    QPushButton *m_btn;
    QProgressBar *m_bar;
    MyThread *m_thread;
};

#endif // MAINWINDOW_H