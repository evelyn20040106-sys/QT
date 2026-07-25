#ifndef MUSICPLAYER_H  // 防止头文件被重复包含的宏守卫开始
#define MUSICPLAYER_H  // 定义宏 MUSICPLAYER_H，标记此头文件已被包含

#include <QObject>        // 引入 QObject 基类，所有 Qt 对象的基类，提供信号/槽机制
#include <QMediaPlayer>   // 引入 QMediaPlayer 类，用于媒体文件的播放控制
#include <QAudioOutput>   // 引入 QAudioOutput 类，用于管理音频输出设备
#include <QUrl>           // 引入 QUrl 类，用于表示和解析统一资源定位符
#include <QVector>        // 引入 QVector 容器类（虽未直接使用，但为扩展预留）
#include <QString>        // 引入 QString 类，用于处理 Unicode 字符串

/**
 * @brief 音乐播放器类，封装 QMediaPlayer 和 QAudioOutput，管理本地播放列表
 *
 * MusicPlayer 继承自 QObject，提供了播放/暂停/切换曲目等基本播放控制功能，
 * 内部维护一个字符串列表作为播放列表，并发射相应的信号通知外部状态变化。
 */
class MusicPlayer : public QObject  // 定义 MusicPlayer 类，公开继承自 QObject
{
    Q_OBJECT  // Qt 元对象编译器（MOC）必需的宏，启用信号/槽、属性等元对象功能
    // 声明 duration 属性，可读（通过 duration()），值变化时发射 durationChanged 信号
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    // 声明 position 属性，可读（通过 position()），值变化时发射 positionChanged 信号
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    // 声明 playbackState 属性，可读（通过 playbackState()），状态变化时发射 playbackStateChanged 信号
    Q_PROPERTY(PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)

public:  // 公开成员开始
    /**
     * @brief 播放状态枚举，映射 QMediaPlayer 的播放状态
     */
    enum PlaybackState {           // 定义播放状态枚举类型
        Stopped = QMediaPlayer::StoppedState,  // 停止状态，对应 QMediaPlayer::StoppedState
        Playing = QMediaPlayer::PlayingState,  // 播放中状态，对应 QMediaPlayer::PlayingState
        Paused  = QMediaPlayer::PausedState    // 暂停状态，对应 QMediaPlayer::PausedState
    };
    Q_ENUM(PlaybackState)  // 注册 PlaybackState 枚举到 Qt 元对象系统，支持枚举的字符串转换

    /**
     * @brief 构造函数
     * @param parent 父级 QObject 指针，默认为 nullptr；负责对象生命周期管理
     */
    explicit MusicPlayer(QObject *parent = nullptr);  // 显式构造函数，防止隐式类型转换

    /**
     * @brief 设置播放列表
     * @param files 音频文件路径的字符串列表
     */
    void setPlaylist(const QStringList &files);  // 设置播放列表的方法声明

    /**
     * @brief 获取当前播放列表
     * @return QStringList 当前存储的播放列表（文件路径列表）
     */
    QStringList playlist() const { return m_playlist; }  // 内联实现，返回私有成员 m_playlist

    /**
     * @brief 按索引播放指定曲目
     * @param index 要播放的曲目在播放列表中的索引
     */
    void playIndex(int index);  // 按索引播放方法声明

    /**
     * @brief 开始或恢复播放
     */
    void play();  // 播放方法声明

    /**
     * @brief 暂停当前播放
     */
    void pause();  // 暂停方法声明

    /**
     * @brief 切换播放/暂停状态
     */
    void togglePlayPause();  // 播放/暂停切换方法声明

    /**
     * @brief 播放下一个曲目（列表循环）
     */
    void next();  // 下一曲方法声明

    /**
     * @brief 播放上一个曲目（列表循环）
     */
    void previous();  // 上一曲方法声明

    /**
     * @brief 设置播放位置
     * @param position 目标播放位置（毫秒）
     */
    void setPosition(qint64 position);  // 设置播放位置方法声明

    /**
     * @brief 设置音量
     * @param volume 音量值（0~100 的整数）
     */
    void setVolume(int volume);  // 设置音量方法声明

    /**
     * @brief 获取当前播放曲目在列表中的索引
     * @return int 当前曲目索引，若无有效曲目则返回 -1
     */
    int currentIndex() const { return m_currentIndex; }  // 内联实现，返回当前索引

    /**
     * @brief 获取当前播放文件的完整路径
     * @return QString 当前曲目的文件路径，若无有效曲目则返回空字符串
     */
    QString currentFile() const;  // 获取当前文件名方法声明

    /**
     * @brief 获取当前媒体的总时长
     * @return qint64 媒体时长（毫秒）
     */
    qint64 duration() const;  // 获取时长方法声明

    /**
     * @brief 获取当前播放位置
     * @return qint64 当前播放进度（毫秒）
     */
    qint64 position() const;  // 获取播放位置方法声明

    /**
     * @brief 获取当前播放状态
     * @return PlaybackState 当前播放状态（Stopped / Playing / Paused）
     */
    PlaybackState playbackState() const;  // 获取播放状态方法声明

signals:  // 信号部分开始
    /**
     * @brief 当前曲目索引变化时发射的信号
     * @param index 新的曲目索引
     */
    void currentIndexChanged(int index);  // 当前索引改变信号声明

    /**
     * @brief 媒体总时长变化时发射的信号
     * @param duration 新的总时长（毫秒）
     */
    void durationChanged(qint64 duration);  // 时长改变信号声明

    /**
     * @brief 播放位置变化时发射的信号（约每秒发射 4 次）
     * @param position 当前的播放位置（毫秒）
     */
    void positionChanged(qint64 position);  // 位置改变信号声明

    /**
     * @brief 播放状态变化时发射的信号
     * @param state 新的播放状态（Stopped / Playing / Paused）
     */
    void playbackStateChanged(PlaybackState state);  // 播放状态改变信号声明

    /**
     * @brief 播放列表加载完成时发射的信号
     * @param count 列表中曲目的总数
     */
    void playlistLoaded(int count);  // 播放列表加载完成信号声明

private slots:  // 私有槽函数部分开始
    /**
     * @brief 处理 QMediaPlayer 媒体状态变化的槽函数
     * @param status 当前媒体状态（如加载中、播放完毕等）
     */
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);  // 媒体状态变化处理槽

    /**
     * @brief 处理 QMediaPlayer 错误事件的槽函数
     * @param error 错误类型
     */
    void onError(QMediaPlayer::Error error);  // 错误处理槽

private:  // 私有成员部分开始
    /**
     * @brief 播放当前索引指向的曲目（内部辅助方法）
     */
    void playCurrent();  // 内部播放当前曲目的辅助方法声明

    QMediaPlayer *m_player;       // 媒体播放器对象指针，负责实际的媒体解码和播放控制
    QAudioOutput *m_audioOutput;  // 音频输出对象指针，负责将音频数据输出到扬声器
    QStringList m_playlist;       // 播放列表，存储所有音频文件的绝对路径字符串
    int m_currentIndex = -1;      // 当前播放曲目的索引，初始化为 -1 表示无效索引
};  // 类定义结束

#endif // MUSICPLAYER_H  // 防止头文件重复包含的宏守卫结束
