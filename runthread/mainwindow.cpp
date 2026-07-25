#include "MainWindow.h"
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_thread(nullptr)
{
    // 创建界面
    QWidget *w = new QWidget(this);
    setCentralWidget(w);

    QVBoxLayout *layout = new QVBoxLayout(w);
    m_bar = new QProgressBar(this);
    m_btn = new QPushButton("开始", this);
    layout->addWidget(m_bar);
    layout->addWidget(m_btn);

    setFixedSize(300, 150);

    // 连接按钮
    connect(m_btn, &QPushButton::clicked, this, &MainWindow::onStart);
}

MainWindow::~MainWindow()
{
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
    }
}

void MainWindow::onStart()
{
    m_btn->setEnabled(false);
    m_bar->setValue(0);

    m_thread = new MyThread(this);

    // 连接信号
    connect(m_thread, &MyThread::progress, this, &MainWindow::onProgress);
    connect(m_thread, &MyThread::done, this, &MainWindow::onDone);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    m_thread->start();  // 启动线程
}

void MainWindow::onProgress(int value)
{
    m_bar->setValue(value);
}

void MainWindow::onDone()
{
    m_btn->setEnabled(true);
}