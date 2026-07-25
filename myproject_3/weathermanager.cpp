#include "weathermanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QDebug>
#include <QThread>

WeatherManager* WeatherManager::m_instance = nullptr;

WeatherManager* WeatherManager::instance()
{
    if (!m_instance) {
        m_instance = new WeatherManager();
    }
    return m_instance;
}

WeatherManager::WeatherManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_timer(nullptr)
{
    qDebug() << "WeatherManager 创建, 线程ID:" << QThread::currentThreadId();

    // 创建网络管理器
    m_networkManager = new QNetworkAccessManager(this);

    // 使用带参数的信号连接
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WeatherManager::onNetworkReply);  // 现在参数匹配了

    // 创建定时器
    m_timer = new QTimer(this);
    m_timer->setInterval(30 * 60 * 1000);
    connect(m_timer, &QTimer::timeout, this, &WeatherManager::onTimeout);
    m_timer->start();

    // 延迟获取天气
    QTimer::singleShot(100, this, &WeatherManager::fetchWeather);
}
/**
 * @brief 析构函数
 */
WeatherManager::~WeatherManager()
{
    qDebug() << "WeatherManager 销毁";
    m_instance = nullptr;
}

/**
 * @brief 获取最新的天气数据
 */
WeatherData WeatherManager::getLatestWeather() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentWeather;
}

/**
 * @brief 手动刷新天气
 */
void WeatherManager::refreshWeather()
{
    qDebug() << "手动刷新天气";
    fetchWeather();
}

/**
 * @brief 定时器超时处理
 */
void WeatherManager::onTimeout()
{
    qDebug() << "定时刷新天气";
    fetchWeather();
}

/**
 * @brief 发送天气请求
 *
 * 使用和风天气API
 */
void WeatherManager::fetchWeather()
{
    qDebug() << "\n========== 开始获取天气 ==========";
    qDebug() << "当前线程ID:" << QThread::currentThreadId();
    qDebug() << "API Key:" << m_apiKey;
    qDebug() << "API Host:" << m_apiHost;
    qDebug() << "City ID:" << m_cityId;

    // 构建API请求URL
    QString url = QString("https://%1/v7/weather/now?location=%2&key=%3")
                      .arg(m_apiHost, m_cityId, m_apiKey);

    qDebug() << "完整请求URL:" << url;

    // 创建网络请求
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Qt Weather App/1.0");

    // 发送网络请求
    m_networkManager->get(request);

    qDebug() << "网络请求已发送，等待响应...";
}

/**
 * @brief 网络请求完成处理
 */
void WeatherManager::onNetworkReply(QNetworkReply *reply)
{
    qDebug() << "\n========== 收到网络响应 ==========";
    qDebug() << "当前线程ID:" << QThread::currentThreadId();

    if (!reply) {
        qDebug() << "错误: reply 为空指针";
        return;
    }

    qDebug() << "回复对象指针:" << reply;
    qDebug() << "回复对象的元对象类名:" << reply->metaObject()->className();

    // 获取HTTP状态码
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP状态码:" << statusCode;

    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("网络错误: %1 (代码: %2)")
                               .arg(reply->errorString())
                               .arg(reply->error());
        qDebug() << errorMsg;

        // 读取错误响应内容
        QByteArray errorData = reply->readAll();
        qDebug() << "错误响应内容:" << errorData;

        emit weatherError(errorMsg);
        reply->deleteLater();
        return;
    }

    // 读取响应数据
    QByteArray data = reply->readAll();
    qDebug() << "响应数据大小:" << data.size() << "字节";
    qDebug() << "响应内容:" << data;

    // 解析天气数据
    WeatherData weather = parseWeatherData(data);

    if (weather.isValid) {
        // 更新当前天气数据
        {
            QMutexLocker locker(&m_mutex);
            m_currentWeather = weather;
        }

        // 发送更新信号
        emit weatherUpdated(weather);

        qDebug() << "✅ 天气更新成功:" << weather.city
                 << weather.weather << weather.temperature;
    } else {
        QString errorMsg = "❌ 解析天气数据失败";
        qDebug() << errorMsg;
        emit weatherError(errorMsg);
    }

    reply->deleteLater();
}

/**
 * @brief 解析JSON天气数据
 */
WeatherData WeatherManager::parseWeatherData(const QByteArray &data)
{
    WeatherData weather;

    // 解析JSON
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "JSON解析失败";
        return weather;
    }

    QJsonObject obj = doc.object();

    // 检查API返回码
    QString code = obj.value("code").toString();
    if (code != "200") {
        qDebug() << "API错误码:" << code;
        return weather;
    }

    // 解析实时天气数据
    QJsonObject now = obj.value("now").toObject();

    weather.city = "北京";  // 固定为北京
    weather.weather = now.value("text").toString();
    weather.temperature = now.value("temp").toString() + "°C";
    weather.humidity = now.value("humidity").toString() + "%";

    QString windDir = now.value("windDir").toString();
    QString windScale = now.value("windScale").toString();
    weather.wind = windDir + " " + windScale + "级";

    // 解析更新时间
    QString updateTime = obj.value("updateTime").toString();
    QDateTime dt = QDateTime::fromString(updateTime, Qt::ISODate);
    weather.updateTime = dt.toString("hh:mm");

    weather.isValid = true;

    return weather;
}
