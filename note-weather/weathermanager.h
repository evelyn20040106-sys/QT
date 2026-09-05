#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonObject>
#include <QMutex>

struct WeatherData {
    QString city;           // 城市名
    QString weather;        // 天气状况
    QString temperature;    // 温度
    QString humidity;       // 湿度
    QString wind;          // 风向风力
    QString updateTime;     // 更新时间
    bool isValid;           // 数据是否有效

    WeatherData() : isValid(false) {}
};

class WeatherManager : public QObject
{
    Q_OBJECT

public:
    static WeatherManager* instance();  // 单例模式
    ~WeatherManager();
    explicit WeatherManager(QObject *parent = nullptr);
    // 获取最新的天气数据
    WeatherData getLatestWeather() const;

    // 手动刷新天气
    void refreshWeather();

signals:
    // 天气数据更新信号
    void weatherUpdated(const WeatherData &weather);
    // 天气请求错误信号
    void weatherError(const QString &error);

private slots:
    void onNetworkReply(QNetworkReply *reply);// 网络请求完成
    void onTimeout();// 定时器超时

private:
    // 解析JSON数据
    WeatherData parseWeatherData(const QByteArray &data);

    // 发送天气请求
    void fetchWeather();

    static WeatherManager* m_instance;

    QNetworkAccessManager *m_networkManager;  // 网络管理器
    QTimer *m_timer;                           // 定时器
    WeatherData m_currentWeather;               // 当前天气数据
    mutable QMutex m_mutex;                     // 互斥锁，保护共享数据

    // 和风天气API配置
    const QString m_apiKey = "50e0bb4f3fc84fd3b73052ac686b9cd1";
    const QString m_apiHost = "n44y3kmedh.re.qweatherapi.com";
    const QString m_cityId = "101010100";        // 北京城市ID
};

#endif
