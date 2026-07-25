#include <QThread>
#include <QTimer>
#include "worker.h"

worker::worker(QObject *parent) : QObject(parent){}

void worker::dowork(int fileCount)
{
    qDebug() << "worker::dowork 运行在线程：" << QThread::currentThreadId();

    emit statusUpdated("开始处理...");

    for(int i = 0; i < fileCount; i++)
    {
        processFile(i);

        int percent = ((i + 1) * 100) / fileCount;

        emit progressUpdated(percent);

        emit statusUpdated(QString("正在处理文件 %1 / %2").arg(i + 1).arg(fileCount));

        QThread::msleep(500);

        qDebug() << "worker 任务结束";
    }
}

void worker::processFile(int fileIndex)
{
    qDebug() << "正在处理文件" << fileIndex;
}