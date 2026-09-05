#ifndef WEATHERTHREAD_H
#define WEATHERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "weathermanager.h"

class WeatherThread : public QThread
{
    Q_OBJECT

public:
    explicit WeatherThread(QObject *parent = nullptr);
    ~WeatherThread();

    // 获取最新的天气数据
    WeatherData getLatestWeather();

    // 请求刷新天气
    void requestRefresh();

signals:
    // 天气更新信号（跨线程）
    void weatherUpdated(const WeatherData &weather);
    // 错误信号
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    WeatherManager *m_weatherManager;
    QMutex m_mutex;
    bool m_stop;
};

#endif // WEATHERTHREAD_H
