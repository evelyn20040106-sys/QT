#include "MainWindow.h"
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_worker(nullptr)
{
    setupUI();

    m_worker = new worker();
    m_worker->moveToThread(&m_workerThread);

    connect(this, &MainWindow::startWork, m_worker, &worker::dowork);
    connect(m_worker, &worker::progressUpdated, this, &MainWindow::updateProgress);
    connect(m_worker, &worker::statusUpdated, this, &MainWindow::updateStatus);
    connect(m_worker, &worker::workFinished, this, &MainWindow::onWorkerFinished);

    m_workerThread.start();

    qDebug() << "主线程 ID" << QThread::currentThreadId();
}

MainWindow::~MainWindow()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

void MainWindow::setupUI()
{
    setFixedSize(400, 200);
    auto *w = new QWidget(this);
    setCentralWidget(w);
    auto *layout = new QVBoxLayout(w);

    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    m_startButton = new QPushButton("开始处理10个文件", this);

    layout->addWidget(m_statusLabel);
    layout->addWidget(m_progressBar);
    layout->addWidget(m_startButton);

    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
}

void MainWindow::onStartButtonClicked()
{
    m_startButton->setEnabled(false);
    m_progressBar->setValue(0);
    m_statusLabel->setText("准备启动...");

    // 发射信号，触发 Worker 在子线程中执行 doWork
    emit startWork(10);
}

void MainWindow::updateProgress(int percent)
{
    m_progressBar->setValue(percent);
}

void MainWindow::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void MainWindow::onWorkerFinished()
{
    m_startButton->setEnabled(true);
    QMessageBox::information(this, "完成", "所有文件处理完成！");
}


