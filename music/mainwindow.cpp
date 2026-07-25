#include "mainwindow.h"     // 包含主窗口头文件，声明了 MainWindow 类

#include <QVBoxLayout>     // 包含垂直布局管理器 QVBoxLayout
#include <QHBoxLayout>     // 包含水平布局管理器 QHBoxLayout
#include <QFileDialog>     // 包含文件对话框类 QFileDialog，用于打开目录选择对话框
#include <QFileInfo>       // 包含文件信息类 QFileInfo，用于提取文件名和扩展名
#include <QStyle>          // 包含 Qt 样式接口 QStyle，用于获取系统标准图标

/**
 * @brief 构造函数：初始化主窗口及其子组件
 * @param parent 父窗口部件
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)               // 调用基类 QMainWindow 的构造函数
    , m_player(new MusicPlayer(this))   // 创建 MusicPlayer 播放器对象，以当前窗口为父对象
    , m_scanner(new MusicScanner(this)) // 创建 MusicScanner 扫描器对象，以当前窗口为父对象
    , m_splitter(nullptr)               // 初始化分割器指针为空，待 setupUI 中创建
    , m_visualizer(nullptr)             // 初始化可视化组件指针为空，待 setupUI 中创建
{
    setupUI();  // 调用 setupUI 创建所有 UI 组件并完成布局

    // 连接播放器信号：播放位置变化时更新界面
    connect(m_player, &MusicPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    // 连接播放器信号：音频总时长变化时更新界面
    connect(m_player, &MusicPlayer::durationChanged, this, &MainWindow::onDurationChanged);
    // 连接播放器信号：播放状态变化时更新播放/暂停按钮图标
    connect(m_player, &MusicPlayer::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    // 连接播放器信号：当前曲目索引变化时同步高亮播放列表
    connect(m_player, &MusicPlayer::currentIndexChanged, this, &MainWindow::onCurrentIndexChanged);

    // 连接扫描器信号：扫描完成后加载文件到播放列表
    connect(m_scanner, &MusicScanner::scanFinished, this, &MainWindow::onScanFinished);

    // 连接播放列表控件信号：用户切换行时触发播放
    connect(m_playlistWidget, &QListWidget::currentRowChanged, this, &MainWindow::onPlaylistItemClicked);
}

/**
 * @brief 初始化用户界面：创建所有控件并完成布局和样式设置
 */
void MainWindow::setupUI()
{
    // 创建中心部件，所有其他控件都将放置在其中
    auto *central = new QWidget(this);
    // 将刚刚创建的 QWidget 设为主窗口的中心部件
    setCentralWidget(central);

    // 创建垂直布局作为主布局，附加在 central 部件上
    auto *mainLayout = new QVBoxLayout(central);
    // 设置主布局的外边距为 10 像素（左、上、右、下）
    mainLayout->setContentsMargins(10, 10, 10, 10);
    // 设置主布局内子控件之间的间距为 8 像素
    mainLayout->setSpacing(8);

    // 设置窗口标题为 "音乐播放器"
    setWindowTitle("音乐播放器");

    // -------------------- 顶部栏（选择目录 + 歌曲名称） --------------------

    // 创建水平布局用于放置顶部控件
    auto *topBar = new QHBoxLayout;
    // 创建 "选择音乐文件夹" 按钮
    m_btnSelectDir = new QPushButton("选择音乐文件夹");
    // 设置按钮的样式表：红色背景、白色文字、无边框、圆角等
    m_btnSelectDir->setStyleSheet(R"(
        QPushButton {
            background: #e94560; color: white; border: none;
            padding: 8px 20px; border-radius: 5px; font-size: 14px;
        }
        QPushButton:hover { background: #c73e54; }
    )");
    // 将目录选择按钮添加到顶部栏的左侧
    topBar->addWidget(m_btnSelectDir);
    // 添加弹性伸缩空间，将按钮和歌曲名称标签分隔开
    topBar->addStretch();

    // 创建显示当前歌曲名称的标签，初始显示 "未选择歌曲"
    m_labelSongName = new QLabel("未选择歌曲");
    // 设置歌曲名称标签的样式：浅色文字、大字号、加粗
    m_labelSongName->setStyleSheet("color: #eee; font-size: 16px; font-weight: bold;");
    // 设置标签文字居中对齐
    m_labelSongName->setAlignment(Qt::AlignCenter);
    // 将歌曲名称标签添加到顶部栏的中间位置
    topBar->addWidget(m_labelSongName);
    // 再次添加弹性伸缩空间，使标签也居于右侧弹性空间之前
    topBar->addStretch();

    // 将顶部栏水平布局添加到主垂直布局中
    mainLayout->addLayout(topBar);

    // -------------------- 分割器：播放列表 | 可视化组件 --------------------

    // 创建水平方向的分割器，用于左右分隔播放列表和可视化区域
    m_splitter = new QSplitter(Qt::Horizontal);

    // --- 左侧：播放列表 ---

    // 创建播放列表控件（QListWidget）
    m_playlistWidget = new QListWidget;
    // 设置播放列表的样式表：深蓝背景、圆角、浅色文字等
    m_playlistWidget->setStyleSheet(R"(
        QListWidget {
            background: #16213e; border: none; border-radius: 8px;
            color: #ccc; font-size: 13px; padding: 5px;
        }
        QListWidget::item { padding: 6px 10px; border-radius: 4px; }
        QListWidget::item:selected { background: #0f3460; color: white; }
        QListWidget::item:hover { background: #1a1a4e; }
    )");
    // 设置播放列表的最小宽度为 200 像素
    m_playlistWidget->setMinimumWidth(200);
    // 将播放列表控件添加到分割器的左侧
    m_splitter->addWidget(m_playlistWidget);

    // --- 右侧：音频可视化组件 ---

    // 创建音频可视化控件
    m_visualizer = new VisualizerWidget;
    // 将可视化控件添加到分割器的右侧
    m_splitter->addWidget(m_visualizer);

    // 设置分割器中第一个控件（播放列表）的拉伸系数为 1
    m_splitter->setStretchFactor(0, 1);
    // 设置分割器中第二个控件（可视化）的拉伸系数为 3，使其占更大比例
    m_splitter->setStretchFactor(1, 3);
    // 将分割器添加到主布局中，第二个参数 1 表示拉伸因子，使分割器占据垂直方向尽可能多的空间
    mainLayout->addWidget(m_splitter, 1);

    // -------------------- 进度条区域（当前时间 + 进度滑块 + 总时长） --------------------

    // 创建水平布局用于放置进度条组件
    auto *progressLayout = new QHBoxLayout;
    // 创建显示当前播放时间的标签，初始为 "00:00"
    m_labelCurrentTime = new QLabel("00:00");
    // 设置当前时间标签的样式：灰色小字
    m_labelCurrentTime->setStyleSheet("color: #aaa; font-size: 12px;");
    // 固定当前时间标签的宽度为 45 像素，保证对齐
    m_labelCurrentTime->setFixedWidth(45);

    // 创建水平方向的进度滑块
    m_progressSlider = new QSlider(Qt::Horizontal);
    // 设置进度滑块的取值范围为 0 ~ 100（初始占位，之后会随实际音频时长更新）
    m_progressSlider->setRange(0, 100);
    // 设置进度滑块的样式表：自定义轨道、滑块和已播放区域的颜色
    m_progressSlider->setStyleSheet(R"(
        QSlider::groove:horizontal {
            height: 6px; background: #333; border-radius: 3px;
        }
        QSlider::handle:horizontal {
            width: 14px; height: 14px; margin: -5px 0;
            background: #e94560; border-radius: 7px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #e94560, stop:1 #0f3460);
            border-radius: 3px;
        }
    )");
    // 将当前时间标签添加到进度水平布局的最左侧
    progressLayout->addWidget(m_labelCurrentTime);
    // 将进度滑块添加到进度水平布局的中间，第二个参数 1 表示拉伸因子，使其占据尽可能多的宽度
    progressLayout->addWidget(m_progressSlider, 1);

    // 创建显示音频总时长的标签，初始为 "00:00"
    m_labelTotalTime = new QLabel("00:00");
    // 设置总时长标签的样式：灰色小字
    m_labelTotalTime->setStyleSheet("color: #aaa; font-size: 12px;");
    // 固定总时长标签的宽度为 45 像素，保证对齐
    m_labelTotalTime->setFixedWidth(45);

    // 将总时长标签添加到进度水平布局的最右侧
    progressLayout->addWidget(m_labelTotalTime);
    // 将整个进度水平布局添加到主垂直布局中
    mainLayout->addLayout(progressLayout);

    // -------------------- 底部控制栏（播放控制按钮 + 音量控制） --------------------

    // 创建水平布局用于放置播放控制按钮和音量控件
    auto *controlLayout = new QHBoxLayout;
    // 设置控制栏内各控件之间的间距为 12 像素
    controlLayout->setSpacing(12);
    // 在控制栏最左侧添加弹性空间，使控件整体居中
    controlLayout->addStretch();

    // 创建"上一曲"按钮
    m_btnPrev = new QPushButton;
    // 设置按钮图标为 Qt 标准图标中的"向后跳过"图标
    m_btnPrev->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    // 固定按钮尺寸为 40x40 像素
    m_btnPrev->setFixedSize(40, 40);
    // 设置"上一曲"按钮的样式：无边框，悬停时显示深色圆形背景
    m_btnPrev->setStyleSheet("QPushButton { border: none; } QPushButton:hover { background: #333; border-radius: 20px; }");

    // 创建"播放/暂停"按钮
    m_btnPlayPause = new QPushButton;
    // 设置按钮图标为 Qt 标准图标中的"播放"图标
    m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    // 固定按钮尺寸为 50x50 像素（比前后按钮稍大，突出其重要性）
    m_btnPlayPause->setFixedSize(50, 50);
    // 设置按钮样式：红色圆形边框，悬停时背景变暗
    m_btnPlayPause->setStyleSheet("QPushButton { border: 2px solid #e94560; border-radius: 25px; } QPushButton:hover { background: #333; }");

    // 创建"下一曲"按钮
    m_btnNext = new QPushButton;
    // 设置按钮图标为 Qt 标准图标中的"向前跳过"图标
    m_btnNext->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    // 固定按钮尺寸为 40x40 像素
    m_btnNext->setFixedSize(40, 40);
    // 设置"下一曲"按钮的样式：无边框，悬停时显示深色圆形背景
    m_btnNext->setStyleSheet("QPushButton { border: none; } QPushButton:hover { background: #333; border-radius: 20px; }");

    // 将"上一曲"按钮添加到控制栏
    controlLayout->addWidget(m_btnPrev);
    // 将"播放/暂停"按钮添加到控制栏
    controlLayout->addWidget(m_btnPlayPause);
    // 将"下一曲"按钮添加到控制栏
    controlLayout->addWidget(m_btnNext);

    // --- 音量控制 ---

    // 在播放按钮和音量控件之间添加 30 像素的间距
    controlLayout->addSpacing(30);
    // 创建"音量"文字标签
    auto *volLabel = new QLabel("音量");
    // 设置音量标签的样式：灰色小字
    volLabel->setStyleSheet("color: #aaa; font-size: 12px;");
    // 将音量标签添加到控制栏
    controlLayout->addWidget(volLabel);

    // 创建水平方向的音量滑块
    m_volumeSlider = new QSlider(Qt::Horizontal);
    // 设置音量滑块的取值范围为 0 ~ 100（代表 0% ~ 100%）
    m_volumeSlider->setRange(0, 100);
    // 设置音量滑块的初始值为 50（即默认 50% 音量）
    m_volumeSlider->setValue(50);
    // 固定音量滑块的宽度为 100 像素
    m_volumeSlider->setFixedWidth(100);
    // 设置音量滑块的样式表：细轨道、小滑块、红色已填充区域
    m_volumeSlider->setStyleSheet(R"(
        QSlider::groove:horizontal { height: 4px; background: #444; border-radius: 2px; }
        QSlider::handle:horizontal { width: 12px; height: 12px; margin: -4px 0; background: #e94560; border-radius: 6px; }
        QSlider::sub-page:horizontal { background: #e94560; border-radius: 2px; }
    )");
    // 将音量滑块添加到控制栏
    controlLayout->addWidget(m_volumeSlider);
    // 在控制栏最右侧添加弹性空间，使控件整体居中
    controlLayout->addStretch();

    // 将整个控制栏水平布局添加到主垂直布局中
    mainLayout->addLayout(controlLayout);

    // -------------------- 信号与槽连接 --------------------

    // 连接"选择目录"按钮的点击信号到 onSelectDirectory 槽
    connect(m_btnSelectDir, &QPushButton::clicked, this, &MainWindow::onSelectDirectory);
    // 连接"上一曲"按钮的点击信号到播放器的 previous 方法
    connect(m_btnPrev, &QPushButton::clicked, m_player, &MusicPlayer::previous);
    // 连接"播放/暂停"按钮的点击信号到播放器的 togglePlayPause 方法
    connect(m_btnPlayPause, &QPushButton::clicked, m_player, &MusicPlayer::togglePlayPause);
    // 连接"下一曲"按钮的点击信号到播放器的 next 方法
    connect(m_btnNext, &QPushButton::clicked, m_player, &MusicPlayer::next);

    // 连接进度滑块的 sliderMoved 信号（用户拖动滑块时触发）到匿名 Lambda 表达式
    connect(m_progressSlider, &QSlider::sliderMoved, this, [this](int value) {
        m_player->setPosition(value);  // 将滑块的值设为播放器的播放位置
    });

    // 连接音量滑块的 valueChanged 信号到播放器的 setVolume 方法
    connect(m_volumeSlider, &QSlider::valueChanged, m_player, &MusicPlayer::setVolume);

    // 设置主窗口整体样式表：深色背景、浅色文字
    setStyleSheet(R"(
        QMainWindow { background: #1a1a2e; }
        QLabel { color: #eee; }
    )");
}

// ============================
// 槽函数实现
// ============================

/**
 * @brief 槽：打开系统目录选择对话框，让用户选择音乐文件夹
 */
void MainWindow::onSelectDirectory()
{
    // 弹出"选择文件夹"对话框，初始目录为上一次选择的目录
    QString dir = QFileDialog::getExistingDirectory(this, "选择音乐文件夹", m_lastDirectory);
    // 如果用户取消了选择（返回空字符串），则直接返回
    if (dir.isEmpty())
        return;

    // 保存本次选择的目录，供下次打开对话框时记忆
    m_lastDirectory = dir;
    // 清空播放列表中的所有项
    m_playlistWidget->clear();
    // 启动扫描器，异步扫描所选目录下的音乐文件
    m_scanner->startScan(dir);
}

/**
 * @brief 槽：音乐文件扫描完成后的处理
 * @param files 扫描到的所有音乐文件的绝对路径列表
 */
void MainWindow::onScanFinished(const QStringList &files)
{
    // 如果扫描结果为空（没有找到音乐文件）
    if (files.isEmpty()) {
        // 在播放列表中显示一条提示信息
        m_playlistWidget->addItem("(未找到音乐文件)");
        return;  // 直接返回，不再执行后续逻辑
    }

    // 将扫描到的文件路径列表设置给播放器作为播放列表
    m_player->setPlaylist(files);

    // 遍历所有文件路径，构造显示文本并添加到播放列表控件
    for (const QString &file : files) {
        QFileInfo fi(file);                    // 创建 QFileInfo 对象以解析文件路径
        QString display = QString("%1 - %2")   // 构造显示文本，格式为"文件名 - 扩展名"
                              .arg(fi.completeBaseName(), fi.suffix());  // 填入文件主名和扩展名
        m_playlistWidget->addItem(display);    // 将格式化后的文本添加到播放列表控件
    }

    // 如果文件列表非空
    if (!files.isEmpty()) {
        m_player->playIndex(0);  // 自动从第一首开始播放
    }
}

/**
 * @brief 槽：播放列表项被点击（切换行）时的处理
 * @param row 被点击的行的索引，-1 表示取消选择
 */
void MainWindow::onPlaylistItemClicked(int row)
{
    // 仅当行号有效（大于等于 0）时执行播放
    if (row >= 0) {
        m_player->playIndex(row);  // 通知播放器播放指定索引的曲目
    }
}

/**
 * @brief 槽：播放位置发生变化时的处理
 * @param position 当前播放位置，单位为毫秒
 */
void MainWindow::onPositionChanged(qint64 position)
{
    // 仅当用户没有正在拖拽进度滑块时，才同步更新滑块位置（避免拖拽时跳动）
    if (!m_progressSlider->isSliderDown()) {
        m_progressSlider->setValue(static_cast<int>(position));  // 将滑块值设为当前播放位置
    }
    // 将当前播放位置格式化为 mm:ss 并更新到时间标签
    m_labelCurrentTime->setText(formatTime(position));
}

/**
 * @brief 槽：音频总时长发生变化时的处理
 * @param duration 音频总时长，单位为毫秒
 */
void MainWindow::onDurationChanged(qint64 duration)
{
    // 根据音频总时长更新进度滑块的取值范围（0 ~ duration）
    m_progressSlider->setRange(0, static_cast<int>(duration));
    // 将总时长格式化为 mm:ss 并更新到总时长标签
    m_labelTotalTime->setText(formatTime(duration));
}

/**
 * @brief 槽：播放状态发生变化时的处理
 * @param state 新的播放状态（Playing / Paused / Stopped）
 */
void MainWindow::onPlaybackStateChanged(MusicPlayer::PlaybackState state)
{
    // 根据当前播放状态执行不同的界面更新
    switch (state) {
    case MusicPlayer::Playing:                         // 如果状态变为"播放中"
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));  // 按钮图标切换为"暂停"
        if (m_visualizer) m_visualizer->setActive(true);  // 激活可视化动画
        break;
    case MusicPlayer::Paused:                          // 如果状态变为"已暂停"
    case MusicPlayer::Stopped:                         // 或状态变为"已停止"
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));   // 按钮图标切换为"播放"
        if (m_visualizer) m_visualizer->setActive(false); // 停止可视化动画
        break;
    }
}

/**
 * @brief 槽：当前播放曲目索引发生变化时的处理
 * @param index 新的曲目索引
 */
void MainWindow::onCurrentIndexChanged(int index)
{
    // 如果索引无效（小于 0 或超过播放列表项数），直接返回
    if (index < 0 || index >= m_playlistWidget->count())
        return;

    // 高亮播放列表中对应的行
    m_playlistWidget->setCurrentRow(index);
    // 滚动播放列表，使当前播放行可见
    m_playlistWidget->scrollToItem(m_playlistWidget->item(index));

    // 获取当前正在播放的文件的绝对路径
    QString filePath = m_player->currentFile();
    // 如果文件路径不为空，则更新歌曲名称标签
    if (!filePath.isEmpty()) {
        QFileInfo fi(filePath);                    // 创建 QFileInfo 解析路径
        m_labelSongName->setText(fi.completeBaseName());  // 仅显示文件名（不含扩展名）
    }
}

// ============================
// 辅助函数实现
// ============================

/**
 * @brief 将毫秒数格式化为 "mm:ss" 格式的字符串
 * @param ms 毫秒数
 * @return 格式化后的时间字符串，例如 230000ms 返回 "03:50"
 */
QString MainWindow::formatTime(qint64 ms) const
{
    int totalSec = static_cast<int>(ms / 1000);  // 将毫秒转换为总秒数
    int min = totalSec / 60;                     // 计算整分钟数
    int sec = totalSec % 60;                     // 计算剩余秒数
    // 返回格式化的时间字符串，分钟和秒均占 2 位，不足补零
    return QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));
}
