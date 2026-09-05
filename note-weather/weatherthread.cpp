#include "weatherthread.h"
#include <QDebug>

WeatherThread::WeatherThread(QObject *parent)
    : QThread(parent)
    , m_weatherManager(nullptr)
    , m_stop(false)
{
    qDebug() << "WeatherThread 创建";
}

WeatherThread::~WeatherThread()
{
    qDebug() << "WeatherThread 销毁";

    m_stop = true;

    // 退出事件循环
    quit();

    // 等待线程结束
    wait();

    // WeatherManager 会被自动删除，因为它是 QObject
}

/**
 * @brief 线程入口函数
 */
void WeatherThread::run()
{
    qDebug() << "天气线程启动";

    // 在工作线程中创建WeatherManager
    // 注意：不能使用单例，因为要在工作线程中创建
    m_weatherManager = new WeatherManager();

    // 连接信号（跨线程连接会自动排队）
    connect(m_weatherManager, &WeatherManager::weatherUpdated,
            this, &WeatherThread::weatherUpdated);
    connect(m_weatherManager, &WeatherManager::weatherError,
            this, &WeatherThread::errorOccurred);

    // 进入事件循环
    exec();

    // 清理
    delete m_weatherManager;
    m_weatherManager = nullptr;

    qDebug() << "天气线程结束";
}

/**
 * @brief 获取最新的天气数据
 */
WeatherData WeatherThread::getLatestWeather()
{
    if (m_weatherManager) {
        return m_weatherManager->getLatestWeather();
    }

    WeatherData invalid;
    invalid.isValid = false;
    return invalid;
}

/**
 * @brief 请求刷新天气
 */
void WeatherThread::requestRefresh()
{
    if (m_weatherManager) {
        // 通过信号槽方式调用，确保在工作线程中执行
        QMetaObject::invokeMethod(m_weatherManager, "refreshWeather",
                                  Qt::QueuedConnection);
        qDebug() << "请求刷新天气";
    }
}
