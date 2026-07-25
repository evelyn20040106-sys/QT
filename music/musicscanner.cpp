#include "musicscanner.h"   // 包含对应的头文件，引入类声明和类型定义
#include <QThreadPool>      // 引入 QThreadPool，用于在全局线程池中执行任务
#include <QMetaObject>      // 引入 QMetaObject::invokeMethod，用于跨线程调用
#include <QFileInfo>        // 引入 QFileInfo，用于获取文件元信息（后缀名、路径等）
#include <QCoreApplication> // 引入 QCoreApplication::instance()，获取主线程事件循环实例

/**
 * @brief 静态常量定义：支持的音频文件扩展名列表。
 *
 * 目前仅支持 "mp3"，后续可在此列表中追加其他格式（如 "flac", "wav" 等）。
 */
const QStringList ScanTask::s_audioExtensions = {"mp3"};  // 初始化静态成员变量，只包含 mp3

/**
 * @brief 构造函数：保存扫描目录和回调函数。
 * @param directory 待扫描的目录路径
 * @param callback  扫描完成后调用的回调函数
 *
 * 构造函数将传入的目录路径和回调函数保存到成员变量中，
 * 并设置任务执行完成后自动从线程池中销毁。
 */
ScanTask::ScanTask(const QString &directory, Callback callback)
    : m_directory(directory),            // 用初始化列表保存待扫描目录
      m_callback(std::move(callback))    // 用初始化列表转移回调函数所有权（避免拷贝）
{
    setAutoDelete(true);                 // 设置任务执行完毕后自动释放内存
}

/**
 * @brief 重写 QRunnable::run()，在后台线程中执行扫描逻辑。
 *
 * 使用 QDirIterator 递归遍历 m_directory 下的所有文件，
 * 筛选出后缀名匹配 s_audioExtensions 的文件，
 * 将绝对路径收集到 result 列表中，排序后通过回调函数异步传回主线程。
 */
void ScanTask::run()                     // run() 方法，线程池线程的入口函数
{
    QStringList result;                  // 创建一个空的字符串列表，用于存放扫描结果

    /**
     * 创建目录迭代器，遍历 m_directory 下所有文件（包括子目录）。
     * QDir::Files         — 只迭代文件，不包含目录
     * Subdirectories      — 递归进入子目录
     */
    QDirIterator it(
        m_directory,                     // 待遍历的根目录
        QDir::Files,                     // 过滤器：只列出普通文件
        QDirIterator::Subdirectories     // 迭代模式：递归遍历所有子目录
    );

    while (it.hasNext()) {               // 当迭代器中还有下一个条目时，继续循环
        it.next();                       // 移动到下一个条目
        QFileInfo fi = it.fileInfo();    // 获取当前条目的文件信息对象
        QString suffix = fi.suffix()     // 提取文件后缀名（如 "mp3"）
                            .toLower();  // 将后缀名转换为小写，保证大小写不敏感匹配

        /**
         * 检查当前文件的后缀名是否在支持的音频扩展名列表中。
         * 如果匹配，则将文件的绝对路径追加到结果列表中。
         */
        if (s_audioExtensions.contains(suffix)) {   // 判断后缀名是否在列表中
            result.append(fi.absoluteFilePath());   // 将文件的绝对路径加入结果列表
        }
    }

    result.sort();                       // 对结果列表按字典序排序，保证输出顺序稳定

    /**
     * 将扫描结果通过回调函数传回主线程。
     * 使用 QMetaObject::invokeMethod 配合 Qt::QueuedConnection，
     * 确保回调在主线程的事件循环中执行，避免跨线程直接操作 UI 的问题。
     */
    if (m_callback) {                    // 检查回调函数是否有效（非空）
        /**
         * invokeMethod 在 QCoreApplication 的主线程事件循环中执行 lambda。
         * Qt::QueuedConnection 确保 lambda 被投递到主线程的消息队列中执行。
         */
        QMetaObject::invokeMethod(
            QCoreApplication::instance(),    // 目标对象：应用实例（保证在主线程）
            [this, result]() {               // lambda 捕获 this 和 result 的副本
                m_callback(result);          // 调用回调函数，传递扫描结果列表
            },
            Qt::QueuedConnection             // 使用队列连接，跨线程安全地投递调用
        );
    }
}

/**
 * @brief 构造函数。
 * @param parent 父 QObject 对象（可选），用于 Qt 对象树内存管理
 *
 * 将父对象指针转发给 QObject 基类构造函数，建立对象树关系。
 */
MusicScanner::MusicScanner(QObject *parent)
    : QObject(parent)                    // 调用基类构造函数，设置父对象
{
    // 构造函数体为空，无需额外的初始化操作
}

/**
 * @brief 开始扫描指定目录，在后台线程池中异步执行。
 * @param directory 要扫描的目录路径
 *
 * 创建一个 ScanTask 对象，将扫描完成后的结果通过 emit scanFinished 信号发射出去。
 * 任务提交到 QThreadPool::globalInstance() 后，由线程池自动调度执行。
 */
void MusicScanner::startScan(const QString &directory)
{
    /**
     * 创建一个 ScanTask 堆对象，传入要扫描的目录和回调 lambda。
     * 回调 lambda 捕获 this 指针，当扫描完成时发射 scanFinished 信号。
     *
     * 注意：ScanTask 设置了 setAutoDelete(true)，任务执行完后会自动 delete 自身，
     *       因此这里不需要手动管理 task 的生命周期。
     */
    auto *task = new ScanTask(           // 在堆上创建扫描任务
        directory,                       // 传入用户指定的扫描目录
        [this](const QStringList &files) {   // lambda 回调：捕获当前 MusicScanner 对象
            emit scanFinished(files);    // 发射信号，将扫描结果传递给连接的槽函数
        }
    );

    /**
     * 将任务提交到全局 QThreadPool 中。
     * QThreadPool::globalInstance() 返回进程唯一的线程池实例，
     * 它会自动在可用线程上调度执行 task->run()。
     */
    QThreadPool::globalInstance()        // 获取全局线程池实例
        ->start(task);                   // 将任务加入线程池调度队列，立即开始执行
}
