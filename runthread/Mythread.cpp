#include "MyThread.h"
#include <QThread>

MyThread::MyThread(QObject *parent) : QThread(parent) {}

void MyThread::run()
{
    for (int i = 0; i <= 100; i += 10) {
        emit progress(i);      // 发射进度
        QThread::msleep(500);  // 模拟耗时
    }
    emit done();               // 发射完成信号
}