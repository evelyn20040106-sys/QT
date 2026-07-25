#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QList>

struct TodoItem {
    int id;                 // 数据库ID
    QString title;          // 任务标题
    QDateTime deadline;     // 截止日期
    bool completed;         // 是否已完成
};


struct NoteItem {
    int id;                 // 数据库ID
    QString title;          // 笔记标题
    QString content;        // 笔记内容
    QDateTime updatedAt;    // 更新时间
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    // 获取单例实例
    static DatabaseManager* instance();
    ~DatabaseManager();

    // 初始化数据库
    bool initialize();

    QList<TodoItem> getAllTodos();                    // 获取所有待办
    bool addTodo(const QString &title, const QDateTime &deadline);  // 添加待办
    bool updateTodo(int id, const QString &title, const QDateTime &deadline);  // 更新待办
    bool deleteTodo(int id);                          // 删除待办
    bool updateTodoStatus(int id, bool completed);    // 更新完成状态

    QList<NoteItem> getAllNotes();                    // 获取所有笔记
    bool addNote(const QString &title, const QString &content);  // 添加笔记
    bool updateNote(int id, const QString &title, const QString &content);  // 更新笔记
    bool deleteNote(int id);                          // 删除笔记
    QString getNoteContent(int id);                    // 获取笔记内容

private:
    explicit DatabaseManager(QObject *parent = nullptr);

    // 创建数据库表
    bool createTables();

    static DatabaseManager* m_instance;
    QSqlDatabase m_db;
    bool m_initialized;
};
#endif // DATABASEMANAGER_H
