#include "copyworker.h"
#include <QFile>                           // 文件操作核心
#include <QFileInfo>                       // 获取文件大小
#include <QDir>                            // 创建文件夹
#include <QElapsedTimer>                   // 高精度计时
#include <QDebug>

static const int BUFFER_SIZE = 8 * 1024 * 1024;           // 8MB 缓冲区读写
static const int SPEED_UPDATE_INTERVAL = 800;

copyworker::copyworker(const QString &source, const QString &dest,QObject *parent)
    : QObject{parent}, m_source(source), m_dest(dest)
{
    m_speedTimer = new QTimer(this);
    m_speedTimer->setInterval(SPEED_UPDATE_INTERVAL);

    connect(m_speedTimer, &QTimer::timeout,
            [this](){
                double deltaMB = m_lastBytes / (1024.0 * 1024.0);
                m_speedMBps = deltaMB / (SPEED_UPDATE_INTERVAL / 1000.0);
                m_lastBytes = 0;
            }
    );
}

copyworker::~copyworker()
{
    m_speedTimer->stop();
}

//拷贝函数
void copyworker::process()
{
    QFile srcFile(m_source);
    if(!srcFile.open(QIODevice::ReadOnly))
    {
        emit finished(false, "打不开源文件！\n" + srcFile.errorString());
        return;
    }
    QDir().mkpath(QFileInfo(m_dest).absolutePath());//创建不存在的文件路径
    QFile dstFile(m_dest);
    if(!dstFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        srcFile.close();
        emit finished(false, "创建目标文件失败！\n" + dstFile.errorString());
        return;
    }

    qint64 totalBytes = srcFile.size();//源文件大小
    qint64 bytesCopied = 0;//已经拷贝字节
    dstFile.resize(totalBytes);
    char *buffer = new char[BUFFER_SIZE];
    m_speedTimer->start();
    m_lastBytes = 0;
    QElapsedTimer segmentTimer;
    segmentTimer.start();

    while(true)
    {
        qint64 bytesRead = srcFile.read(buffer, BUFFER_SIZE);//读取字节
        if(bytesRead <= 0)
            break;
        if(dstFile.write(buffer, bytesRead) != bytesRead)
        {
            delete[] buffer;
            srcFile.close();
            dstFile.close();
            emit finished(false, "写入失败，磁盘已满，或者损坏");
            return;
        }
        bytesCopied += bytesRead;
        m_lastBytes += bytesRead;

        if (bytesCopied % (64 * 1024 * 1024) < BUFFER_SIZE || segmentTimer.elapsed() > 500)
        {
            emit progress(bytesCopied, totalBytes);
            segmentTimer.restart();
        }
    }

    //结束
    delete[] buffer;
    srcFile.close();
    dstFile.close();
    m_speedTimer->stop();
    //最后检查
    if (QFileInfo(m_dest).size() != totalBytes)
    {
        emit finished(false, "备份完成但文件大小不一致！可能损坏！");
        return;
    }
    emit progress(totalBytes, totalBytes);   // 进度条到100%
    emit finished(true);
}

/*
硬盘/SSD 一次最少喜欢写 4MB–8MB（内部页面对齐）
→ 你给它 7.9MB，它还是要按 8MB 对齐写，浪费了
→ 你给它 8MB，正好一次怼满，效率最高！

操作系统缓存（Page Cache）一般是 4MB 或 8MB 一页
→ 你写 8MB，正好一次系统调用就能怼进缓存，命中率最高！

Qt 的 QFile 底层用的是 POSIX read/write
→ 系统调用是有开销的！
→ 你调 1 万次 8KB = 1 万次系统调用 → CPU 累死
→ 你调 1 次 8MB = 1 次系统调用 → CPU 笑得合不拢嘴！
*/


