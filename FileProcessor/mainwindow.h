#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "worker.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QProgressBar;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

signals:

    void startWork(int fileCount);

private slots:

    void onStartButtonClicked();
    void updateProgress(int percent);
    void updateStatus(const QString &status);
    void onWorkerFinished();

private:

    void setupUI();

    QPushButton *m_startButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    QThread m_workerThread;
    worker *m_worker;
};

#endif