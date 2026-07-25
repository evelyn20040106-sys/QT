#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>        // 引入 QMainWindow 基类，用于创建主窗口
#include <QSlider>            // 引入滑块控件，用于进度条和音量条
#include <QLabel>             // 引入标签控件，用于显示文本信息
#include <QPushButton>        // 引入按钮控件，用于交互操作
#include <QListWidget>        // 引入列表控件，用于显示播放列表
#include <QSplitter>          // 引入分割器控件，用于分隔播放列表和可视化区域
#include <QDir>               // 引入目录操作类，用于处理文件路径
#include "musicplayer.h"      // 引入音乐播放器核心类
#include "musicscanner.h"     // 引入音乐文件扫描器类
#include "visualizerwidget.h" // 引入音频可视化组件类

/**
 * @brief 主窗口类，应用程序的主界面
 *
 * 负责管理用户界面的布局、播放列表显示、播放控制交互，
 * 并协调 MusicPlayer 和 MusicScanner 之间的协作。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT    // 启用 Qt 元对象系统，支持信号与槽机制

public:
    /**
     * @brief 构造函数，初始化主窗口
     * @param parent 父窗口部件，默认为 nullptr
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数，默认实现
     */
    ~MainWindow() override = default;

private slots:
    /**
     * @brief 槽：打开目录选择对话框，选择音乐文件夹
     */
    void onSelectDirectory();

    /**
     * @brief 槽：音乐文件扫描完成后的处理
     * @param files 扫描到的音乐文件路径列表
     */
    void onScanFinished(const QStringList &files);

    /**
     * @brief 槽：播放列表项被点击时的处理
     * @param row 被点击的行号
     */
    void onPlaylistItemClicked(int row);

    /**
     * @brief 槽：播放位置发生变化时的处理
     * @param position 当前播放位置，单位为毫秒
     */
    void onPositionChanged(qint64 position);

    /**
     * @brief 槽：音频总时长发生变化时的处理
     * @param duration 音频总时长，单位为毫秒
     */
    void onDurationChanged(qint64 duration);

    /**
     * @brief 槽：播放状态发生变化时的处理
     * @param state 新的播放状态（Playing / Paused / Stopped）
     */
    void onPlaybackStateChanged(MusicPlayer::PlaybackState state);

    /**
     * @brief 槽：当前播放曲目索引发生变化时的处理
     * @param index 新的曲目索引
     */
    void onCurrentIndexChanged(int index);

private:
    /**
     * @brief 初始化用户界面，创建所有 UI 组件并布局
     */
    void setupUI();

    /**
     * @brief 将毫秒格式化为 "mm:ss" 格式的字符串
     * @param ms 毫秒数
     * @return 格式化后的时间字符串，例如 "03:45"
     */
    QString formatTime(qint64 ms) const;

    // -------------------- UI 组件 --------------------
    QPushButton *m_btnSelectDir;    // "选择音乐文件夹" 按钮
    QPushButton *m_btnPrev;         // 上一曲按钮
    QPushButton *m_btnPlayPause;    // 播放/暂停切换按钮
    QPushButton *m_btnNext;         // 下一曲按钮
    QSlider     *m_progressSlider;  // 播放进度滑块
    QSlider     *m_volumeSlider;    // 音量调节滑块
    QLabel      *m_labelCurrentTime; // 显示当前播放时间的标签
    QLabel      *m_labelTotalTime;  // 显示音频总时长的标签
    QLabel      *m_labelSongName;   // 显示当前歌曲名称的标签
    QListWidget *m_playlistWidget;  // 播放列表面板
    QSplitter   *m_splitter;        // 水平分割器，分隔列表与可视化区域
    VisualizerWidget *m_visualizer; // 音频可视化显示组件

    // -------------------- 核心业务组件 --------------------
    MusicPlayer  *m_player;         // 音乐播放器核心对象
    MusicScanner *m_scanner;        // 音乐文件扫描器对象

    QString m_lastDirectory;        // 上一次打开的目录路径，用于对话框记忆
};

#endif // MAINWINDOW_H
