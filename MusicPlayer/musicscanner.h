#ifndef MUSICSCANNER_H      // 头文件保护宏，防止重复包含
#define MUSICSCANNER_H      // 定义头文件保护宏

#include <QObject>          // 引入 QObject 基类，支持 Qt 信号/槽机制
#include <QRunnable>        // 引入 QRunnable 接口，使任务可在线程池中运行
#include <QStringList>      // 引入 QStringList，用于传递文件路径列表
#include <QDir>             // 引入 QDir，用于目录操作
#include <QDirIterator>     // 引入 QDirIterator，用于遍历目录中的条目
#include <QMutex>           // 引入 QMutex，用于多线程同步（预留）

/**
 * @brief 单个扫描任务，继承 QRunnable，可在 QThreadPool 中运行。
 *
 * ScanTask 封装了一次目录扫描操作，在后台线程中递归查找指定目录下的
 * 音频文件，扫描完成后通过回调函数将结果传回主线程。
 */
class ScanTask : public QRunnable    // ScanTask 继承自 QRunnable，使其可被线程池调度
{
public:                              // 公有成员部分
    /**
     * @brief 回调函数类型定义。
     *
     * 接收一个 QStringList 参数，其中包含扫描到的所有音频文件的绝对路径。
     */
    using Callback = std::function<void(const QStringList &files)>;

    /**
     * @brief 构造函数。
     * @param directory 要扫描的目录路径
     * @param callback  扫描完成后的回调函数
     */
    ScanTask(const QString &directory, Callback callback);

    /**
     * @brief 重写 QRunnable::run()，在线程池线程中执行扫描。
     *
     * 遍历指定目录及其子目录，收集所有匹配音频后缀的文件路径，
     * 排序后通过回调函数异步传回主线程。
     */
    void run() override;             // 重写 run() 方法，线程入口点

private:                             // 私有成员部分
    QString m_directory;             // 存储待扫描的目录路径
    Callback m_callback;             // 存储回调函数对象

    /**
     * @brief 支持的音频文件扩展名列表（静态常量）。
     *
     * 所有匹配的后缀名存储在此列表中，用于过滤非音频文件。
     */
    static const QStringList s_audioExtensions;  // 静态常量，所有实例共享
};

/**
 * @brief 便捷封装类，负责向 QThreadPool 派发扫描任务。
 *
 * MusicScanner 继承自 QObject，可以利用 Qt 的信号与槽机制，
 * 在扫描完成后通过信号将结果通知到主线程。
 */
class MusicScanner : public QObject  // MusicScanner 继承自 QObject，支持信号/槽
{
    Q_OBJECT                         // Qt 元对象宏，启用信号/槽和元对象功能

public:                              // 公有成员部分
    /**
     * @brief 构造函数。
     * @param parent 父 QObject 对象（可选），用于 Qt 对象树内存管理
     */
    explicit MusicScanner(QObject *parent = nullptr);  // explicit 防止隐式类型转换

    /**
     * @brief 开始扫描指定目录（在后台线程池中进行）。
     * @param directory 要扫描的目录路径
     *
     * 创建一个 ScanTask 任务并提交到全局 QThreadPool 中执行，
     * 扫描完成后通过 scanFinished 信号返回结果。
     */
    void startScan(const QString &directory);          // 启动扫描的公有接口

signals:                             // Qt 信号部分
    /**
     * @brief 扫描完成时发射的信号。
     * @param files 扫描到的所有音频文件的绝对路径列表
     *
     * 该信号在主线程中发射，接收方可以直接连接槽函数安全地更新 UI。
     */
    void scanFinished(const QStringList &files);       // 扫描完成信号
};

#endif // MUSICSCANNER_H      // 结束头文件保护宏
