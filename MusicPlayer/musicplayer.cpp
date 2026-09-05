#include "musicplayer.h"  // 包含对应头文件，确保声明与定义一致
#include <QMediaMetaData> // 引入 QMediaMetaData 类，用于访问媒体文件的元数据（如标题、艺术家等）

/**
 * @brief 构造函数：初始化播放器和音频输出，并连接必要的信号与槽
 * @param parent 父级 QObject 指针，用于 Qt 对象树生命周期管理
 */
MusicPlayer::MusicPlayer(QObject *parent)
    : QObject(parent)    // 调用基类 QObject 的构造函数，传入父对象指针
    , m_player(new QMediaPlayer(this))        // 创建 QMediaPlayer 实例，以 this 为父对象自动管理生命周期
    , m_audioOutput(new QAudioOutput(this))   // 创建 QAudioOutput 实例，以 this 为父对象自动管理生命周期
{
    // 将音频输出设备设置到媒体播放器上，使播放器的音频数据路由到该输出
    m_player->setAudioOutput(m_audioOutput);
    // 设置初始音量为 0.5（即 50%，QAudioOutput 的音量范围为 0.0 ~ 1.0）
    m_audioOutput->setVolume(0.5);

    // 连接 QMediaPlayer 的 durationChanged 信号到本类的 durationChanged 信号，透传时长变化
    connect(m_player, &QMediaPlayer::durationChanged, this, &MusicPlayer::durationChanged);
    // 连接 QMediaPlayer 的 positionChanged 信号到本类的 positionChanged 信号，透传位置变化
    connect(m_player, &QMediaPlayer::positionChanged, this, &MusicPlayer::positionChanged);
    // 连接 QMediaPlayer 的 playbackStateChanged 信号，用 Lambda 表达式转换为本类的枚举类型后发射
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        // 将 QMediaPlayer 的原生状态枚举值转换为 MusicPlayer 自定义的 PlaybackState 枚举
        emit playbackStateChanged(static_cast<PlaybackState>(state));
    });
    // 连接 QMediaPlayer 的 mediaStatusChanged 信号到本类的 onMediaStatusChanged 槽函数
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MusicPlayer::onMediaStatusChanged);
    // 连接 QMediaPlayer 的 errorOccurred 信号到本类的 onError 槽函数
    connect(m_player, &QMediaPlayer::errorOccurred, this, &MusicPlayer::onError);
}

/**
 * @brief 设置播放列表
 * @param files 包含音频文件路径的字符串列表
 */
void MusicPlayer::setPlaylist(const QStringList &files)
{
    // 将传入的文件路径列表保存到成员变量 m_playlist 中
    m_playlist = files;
    // 重置当前曲目索引为 -1，表示尚未选中任何曲目
    m_currentIndex = -1;
    // 发射 playlistLoaded 信号，通知外部播放列表已加载完成，参数为列表中的曲目数量
    emit playlistLoaded(files.size());
}

/**
 * @brief 按索引播放指定曲目
 * @param index 要播放的曲目在播放列表中的索引
 */
void MusicPlayer::playIndex(int index)
{
    // 检查索引是否越界（小于 0 或超过列表大小），若是则直接返回不执行任何操作
    if (index < 0 || index >= m_playlist.size())
        return;

    // 更新当前曲目索引为传入的有效索引值
    m_currentIndex = index;
    // 调用内部辅助方法 playCurrent()，实际加载并播放当前索引对应的文件
    playCurrent();
    // 发射 currentIndexChanged 信号，通知外部当前曲目索引已改变
    emit currentIndexChanged(m_currentIndex);
}

/**
 * @brief 开始或恢复播放
 */
void MusicPlayer::play()
{
    // 调用 QMediaPlayer 的 play() 方法开始或恢复播放
    m_player->play();
}

/**
 * @brief 暂停当前播放
 */
void MusicPlayer::pause()
{
    // 调用 QMediaPlayer 的 pause() 方法暂停当前播放
    m_player->pause();
}

/**
 * @brief 切换播放/暂停状态
 */
void MusicPlayer::togglePlayPause()
{
    // 判断当前播放器是否处于播放中状态
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
        // 如果正在播放，则调用 pause() 暂停
        m_player->pause();
    else
        // 如果处于暂停或停止状态，则调用 play() 开始播放
        m_player->play();
}

/**
 * @brief 播放下一个曲目（列表循环）
 */
void MusicPlayer::next()
{
    // 检查播放列表是否为空，若为空则直接返回
    if (m_playlist.isEmpty())
        return;

    // 计算下一曲的索引：当前索引加 1 后对列表大小取模，实现循环播放效果
    int nextIndex = (m_currentIndex + 1) % m_playlist.size();
    // 调用 playIndex 跳转到计算得到的下一曲索引开始播放
    playIndex(nextIndex);
}

/**
 * @brief 播放上一个曲目（列表循环）
 */
void MusicPlayer::previous()
{
    // 检查播放列表是否为空，若为空则直接返回
    if (m_playlist.isEmpty())
        return;

    // 计算上一曲的索引：(当前索引 - 1 + 列表大小) % 列表大小，
    // 加列表大小是为了防止取模运算出现负数，从而实现循环播放效果
    int prevIndex = (m_currentIndex - 1 + m_playlist.size()) % m_playlist.size();
    // 调用 playIndex 跳转到计算得到的上一曲索引开始播放
    playIndex(prevIndex);
}

/**
 * @brief 设置播放位置
 * @param position 目标播放位置，单位为毫秒
 */
void MusicPlayer::setPosition(qint64 position)
{
    // 调用 QMediaPlayer 的 setPosition() 方法跳转到指定毫秒位置
    m_player->setPosition(position);
}

/**
 * @brief 设置音量
 * @param volume 音量值，范围为 0 ~ 100 的整数
 */
void MusicPlayer::setVolume(int volume)
{
    // 将 0~100 的整数音量转换为 0.0~1.0 的浮点数，并设置到 QAudioOutput
    m_audioOutput->setVolume(volume / 100.0);
}

/**
 * @brief 获取当前播放文件的完整路径
 * @return QString 当前曲目的文件路径；若当前索引无效则返回空字符串
 */
QString MusicPlayer::currentFile() const
{
    // 检查当前索引是否在有效范围内（0 到列表大小减 1）
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size())
        // 索引有效，返回播放列表中对应索引的文件路径
        return m_playlist[m_currentIndex];
    // 索引无效（为 -1 或其他越界值），返回空字符串
    return {};
}

/**
 * @brief 获取当前媒体的总时长
 * @return qint64 媒体总时长，单位为毫秒
 */
qint64 MusicPlayer::duration() const
{
    // 委托给 QMediaPlayer 的 duration() 方法获取总时长
    return m_player->duration();
}

/**
 * @brief 获取当前播放位置
 * @return qint64 当前播放进度，单位为毫秒
 */
qint64 MusicPlayer::position() const
{
    // 委托给 QMediaPlayer 的 position() 方法获取当前播放位置
    return m_player->position();
}

/**
 * @brief 获取当前播放状态
 * @return PlaybackState 当前播放状态枚举值（Stopped / Playing / Paused）
 */
MusicPlayer::PlaybackState MusicPlayer::playbackState() const
{
    // 将 QMediaPlayer 的原生播放状态枚举强制转换为 MusicPlayer 自定义枚举并返回
    return static_cast<PlaybackState>(m_player->playbackState());
}

/**
 * @brief 处理 QMediaPlayer 媒体状态变化的槽函数
 * @param status 当前的媒体状态（如 EndOfMedia、LoadedMedia、BufferingMedia 等）
 *
 * 当媒体播放到末尾（EndOfMedia）时，自动调用 next() 切换到下一曲，实现连续播放。
 */
void MusicPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // 判断媒体状态是否为"到达媒体末尾"
    if (status == QMediaPlayer::EndOfMedia) {
        // 若当前曲目播放完毕，自动播放下一个曲目
        next();
    }
}

/**
 * @brief 处理 QMediaPlayer 错误事件的槽函数
 * @param error 错误类型（如 ResourceError、FormatError、NetworkError 等）
 *
 * 当前实现中仅使用 Q_UNUSED 抑制编译器"未使用参数"警告，暂不处理具体错误。
 */
void MusicPlayer::onError(QMediaPlayer::Error error)
{
    // 声明参数 error 未被使用，抑制编译器警告（当前暂不处理错误）
    Q_UNUSED(error);
}

/**
 * @brief 播放当前索引指向的曲目（内部辅助方法）
 *
 * 根据 m_currentIndex 从播放列表中取出对应文件路径，设置到播放器中并开始播放。
 */
void MusicPlayer::playCurrent()
{
    // 检查当前索引是否在有效范围内（0 到列表大小减 1）
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        // 通过 QUrl::fromLocalFile() 将本地文件路径转换为 QUrl，设置到播放器作为媒体源
        m_player->setSource(QUrl::fromLocalFile(m_playlist[m_currentIndex]));
        // 调用 play() 方法开始播放刚刚设置的媒体源
        m_player->play();
    }
}
