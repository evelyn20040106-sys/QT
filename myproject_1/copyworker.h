#ifndef COPYWORKER_H
#define COPYWORKER_H

#include <QObject>
#include <QString>
#include <QTimer>

class copyworker : public QObject
{
    Q_OBJECT
public:
    explicit copyworker(const QString &source, const QString &dest, QObject *parent = nullptr);
    ~copyworker();
    double currentSpeedMBps() const
    {
        return m_speedMBps;
    }
public slots:
    void process();//子线程入口

signals:
    void progress(qint64 bytesCopied, qint64 totalBytes);//更新进度条信号
    void finished(bool success, const QString &errorMsg = "");//拷贝结束信号不分成功失败
private:
    QString m_source;        //源文件路径
    QString m_dest;          //目标路径
    QTimer *m_speedTimer;    // 定时器，每800ms算一次
    qint64 m_lastBytes = 0;  // 上次计算速度时已经拷贝的字节数
    double m_speedMBps = 0.0;// 当前速度（MB/s），实时更新
};

#endif // COPYWORKER_H
