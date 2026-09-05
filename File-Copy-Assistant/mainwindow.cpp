#include "mainwindow.h"
#include <QFileDialog>                     // 文件/文件夹选择对话框
#include <QMessageBox>                     // 弹窗（成功、失败提示）
#include <QHBoxLayout>                     // 水平布局
#include <QVBoxLayout>                     // 垂直布局
#include <QFileInfo>                       // 获取文件信息（文件名、大小等）
#include <QDir>                            // 目录操作（创建文件夹）
#include <QElapsedTimer>// 高精度计时器
#include <QLabel>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}

MainWindow::~MainWindow()
{
    if(workerThread && workerThread->isRunning())
    {
        workerThread->quit();
        workerThread->wait(5000);
    }
}

void MainWindow::setupUi()
{
    auto central = new QWidget(this);
    setCentralWidget(central);

    auto layout = new QVBoxLayout(central);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);


    //源文件选择
    auto sourceLayout = new QHBoxLayout();
    lblSource = new QLabel("源文件：", this);
    editSource = new QLineEdit(this);
    editSource->setReadOnly(true);
    editSource->setPlaceholderText("请选择备份文件夹");
    btnSource = new QPushButton("选择文件", this);

    sourceLayout->addWidget(lblSource);
    sourceLayout->addWidget(editSource);
    sourceLayout->addWidget(btnSource);

    //目标路径选择
    auto targetLayout = new QHBoxLayout();
    lblTarget = new QLabel("备份到：", this);
    editTarget = new QLineEdit(this);
    editTarget->setReadOnly(true);
    editTarget->setPlaceholderText("请选择目标文件夹");
    btnTarget = new QPushButton("目标文件", this);

    targetLayout->addWidget(lblTarget);
    targetLayout->addWidget(editTarget);
    targetLayout->addWidget(btnTarget);

    //进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);//显示百分比
    progressBar->setStyleSheet(
        "QProgressBar { border: 2px solid #aaa; border-radius: 8px; text-align: center; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #00ff95, stop:1 #0068ff); border-radius: 6px; }"
        );//美化

    //开始按钮
    btnStart = new QPushButton("开始备份", this);
    btnStart->setFixedHeight(48);
    btnStart->setEnabled(false);
    btnStart->setStyleSheet(
        "QPushButton { background: #00b46a; color: white; font-size: 18px; font-weight: bold; border-radius: 10px; }"
        "QPushButton:hover:!disabled { background: #00d480; }"
        "QPushButton:disabled { background: #aaaaaa; }"
        );//美化

    //状态提示字
    lblStatus = new QLabel("请选择源文件和目标路径", this);
    lblStatus->setStyleSheet("QLabel { color: #666; font-size: 14px; }");

    layout->addLayout(sourceLayout);
    layout->addLayout(targetLayout);
    layout->addWidget(progressBar);
    layout->addWidget(btnStart);
    layout->addWidget(lblStatus);
    layout->addStretch();

    //信号槽连接
    connect(btnSource, &QPushButton::clicked, this, &MainWindow::onSelectSource);
    connect(btnTarget, &QPushButton::clicked, this, &MainWindow::onSelectTarget);
    connect(btnStart,  &QPushButton::clicked, this, &MainWindow::onStartCopy);
    //检查路径有没有变化，是否能点击按钮
    connect(editSource, &QLineEdit::textChanged, this, &MainWindow::updateStartButtonState);
    connect(editTarget, &QLineEdit::textChanged, this, &MainWindow::updateStartButtonState);

}

//检查按钮是否可点击
void MainWindow::updateStartButtonState()
{
    bool ready = !editSource->text().isEmpty() && !editTarget->text().isEmpty();
    btnStart->setEnabled(ready);

    if (!ready)
    {
        lblStatus->setText("请先选择源文件和目标文件夹");
    }
}


//选择源文件夹
void MainWindow::onSelectSource()
{
    //选择文件
    QString file = QFileDialog::getOpenFileName(
        this,
        "选择需要备份的影视文件（支持几百GB甚至TB级母带）",
        QDir::homePath(),                                           // 默认打开用户目录
        "视频文件 (*.mkv *.mp4 *.mov *.avi *.mxf *.m2ts *.ts *.mpg *.mpeg *.vob *.iso *.img *.r3d *.ari *.braw);;"
        "所有文件 (*.*)");
    if (file.isEmpty())
        return;
    editSource->setText(file);//显示源文件路径
    editSource->setToolTip(file);//鼠标悬停显示完整路径
    updateStartButtonState();
}

//选择目标文件夹
void MainWindow::onSelectTarget()
{
    //选择文件夹
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择备份目标文件夹（建议用高速硬盘或RAID）",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty())
        return;
    editTarget->setText(dir);
    editTarget->setToolTip(dir);
    updateStartButtonState();
}

//拷贝启动
void MainWindow::onStartCopy()
{
    QString src = editSource->text();//源
    QString dstDir = editTarget->text();//目标
    QString dst = dstDir + "/" + QFileInfo(src).fileName();

    //防止疯狂点击
    btnStart->setEnabled(false);
    btnStart->setText("准备启动拷贝引擎...");
    progressBar->setValue(0);
    lblStatus->setText("正在启动多线程拷贝引擎...");

    workerThread = new QThread(this);//创建线程
    worker = new copyworker(src, dst);//工人
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, worker, &copyworker::process);
    connect(worker, &copyworker::progress, this, &MainWindow::onCopyProgress);
    connect(worker, &copyworker::finished, this, &MainWindow::onCopyFinished);
    connect(worker, &copyworker::finished, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    workerThread->start();
    btnStart->setText("备份进行中...");
    lblStatus->setText("拷贝已启动，火力全开！");
}

//接收子线程信号，更新进度条
void MainWindow::onCopyProgress(qint64 bytesCopied, qint64 totalBytes)
{
    if(totalBytes <= 0)
        return;
    int percent = static_cast<int>((bytesCopied * 100LL) / totalBytes);
    progressBar->setValue(percent);

    QString status = QString("正在拷贝... %1%").arg(percent);
    double speed = worker->currentSpeedMBps();
    if (speed > 0)
    {
        status += QString("  |  速度：%1 MB/s").arg(speed, 0, 'f', 1);
    }
    double copiedGB = bytesCopied / (1024.0 * 1024.0 * 1024.0);
    double totalGB  = totalBytes  / (1024.0 * 1024.0 * 1024.0);
    if (totalGB >= 1.0)
    {
        status += QString("  (%1 / %2 GB)")
                      .arg(copiedGB, 0, 'f', 1).arg(totalGB,  0, 'f', 1);
    }
    else
    {
        double copiedMB = bytesCopied / (1024.0 * 1024.0);
        double totalMB  = totalBytes  / (1024.0 * 1024.0);
        status += QString("  (%1 / %2 MB)")
                      .arg(copiedMB, 0, 'f', 1).arg(totalMB,  0, 'f', 1);
    }
    lblStatus->setText(status);
}


//收尾
void MainWindow::onCopyFinished(bool success, const QString &errorMsg)
{
    //终止子进程，释放资源
    if(workerThread)
    {
        workerThread->quit();
        workerThread->wait(3000);
        workerThread->deleteLater();
        workerThread = nullptr;
    }
    worker = nullptr;

    if(success)
    {
        progressBar->setValue(100);
        lblStatus->setText("<font color=#00ff00><b>备份完成！文件已完整保存！</b></font>");
        btnStart->setText("备份成功！");
        btnStart->setEnabled(false);
        QMessageBox::information(this, "成功",
                                 "影视母带备份完成！\n文件完整无损，可安全拔盘！",
                                 QMessageBox::Ok);
    }
    else
    {
        lblStatus->setText("<font color=red><b>备份失败：</b></font>" + errorMsg);
        btnStart->setText("开始备份");
        btnStart->setEnabled(true);
        QMessageBox::critical(this, "失败", "备份失败！\n原因：" + errorMsg);
    }
}
















