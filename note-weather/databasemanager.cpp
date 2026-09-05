#include "databasemanager.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QVariant>

// 初始化静态成员变量
DatabaseManager* DatabaseManager::m_instance = nullptr;


DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}


DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
}


DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        QString connectionName = m_db.connectionName();
        m_db.close();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool DatabaseManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    // 获取应用程序数据存储路径
    QString dataPath = "E:/qt_project/myproject_3";
    if (dataPath.isEmpty()) {
        dataPath = QDir::currentPath() + "/data";
    }

    // 创建数据目录
    QDir dir;
    if (!dir.exists(dataPath)) {
        if (!dir.mkpath(dataPath)) {
            qDebug() << "Failed to create data directory:" << dataPath;
            return false;
        }
    }

    // 数据库文件路径
    QString dbPath = dataPath + "/smartnote.db";
    qDebug() << "Database path:" << dbPath;

    // 创建数据库连接
    m_db = QSqlDatabase::addDatabase("QSQLITE", "smartnote_connection");
    m_db.setDatabaseName(dbPath);

    // 打开数据库
    if (!m_db.open()) {
        qDebug() << "Database open error:" << m_db.lastError().text();
        return false;
    }

    // 创建表
    if (!createTables()) {
        m_db.close();
        return false;
    }

    m_initialized = true;
    qDebug() << "Database initialized successfully";
    return true;
}

bool DatabaseManager::createTables()
{
    // 开始事务
    if (!m_db.transaction()) {
        qDebug() << "Failed to start transaction";
        return false;
    }

    QSqlQuery query(m_db);

    // 创建待办事项表
    QString createTodoTable = R"(
        CREATE TABLE IF NOT EXISTS todos (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            deadline DATETIME NOT NULL,
            completed INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createTodoTable)) {
        qDebug() << "Create todo table error:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    // 创建笔记表
    QString createNoteTable = R"(
        CREATE TABLE IF NOT EXISTS notes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            content TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createNoteTable)) {
        qDebug() << "Create note table error:" << query.lastError().text();
        m_db.rollback();
        return false;
    }

    // 提交事务
    m_db.commit();

    // 检查是否有示例数据，如果没有则添加
    query.exec("SELECT COUNT(*) FROM todos");
    if (query.next() && query.value(0).toInt() == 0) {
        // 添加示例待办
        addTodo("完成项目方案", QDateTime::currentDateTime().addDays(2));
        addTodo("购买办公用品", QDateTime::currentDateTime().addDays(1));
        addTodo("团队会议", QDateTime::currentDateTime().addDays(3));
    }

    query.exec("SELECT COUNT(*) FROM notes");
    if (query.next() && query.value(0).toInt() == 0) {
        // 添加示例笔记
        addNote("项目会议纪要", "2024年3月7日项目会议纪要内容...");
        addNote("学习笔记", "Qt数据库编程学习笔记...");
    }

    return true;
}

QList<TodoItem> DatabaseManager::getAllTodos()
{
    QList<TodoItem> todos;

    if (!m_initialized) {
        return todos;
    }

    QSqlQuery query(m_db);
    query.exec("SELECT id, title, deadline, completed FROM todos ORDER BY deadline");

    while (query.next()) {
        TodoItem item;
        item.id = query.value(0).toInt();
        item.title = query.value(1).toString();
        item.deadline = query.value(2).toDateTime();
        item.completed = query.value(3).toBool();
        todos.append(item);
    }

    return todos;
}

bool DatabaseManager::addTodo(const QString &title, const QDateTime &deadline)
{
    if (!m_initialized) return false;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO todos (title, deadline) VALUES (:title, :deadline)");
    query.bindValue(":title", title);
    query.bindValue(":deadline", deadline);

    return query.exec();
}

bool DatabaseManager::updateTodo(int id, const QString &title, const QDateTime &deadline)
{
    if (!m_initialized) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE todos SET title = :title, deadline = :deadline WHERE id = :id");
    query.bindValue(":title", title);
    query.bindValue(":deadline", deadline);
    query.bindValue(":id", id);

    return query.exec();
}

bool DatabaseManager::deleteTodo(int id)
{
    if (!m_initialized) return false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM todos WHERE id = :id");
    query.bindValue(":id", id);

    return query.exec();
}

bool DatabaseManager::updateTodoStatus(int id, bool completed)
{
    if (!m_initialized) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE todos SET completed = :completed WHERE id = :id");
    query.bindValue(":completed", completed ? 1 : 0);
    query.bindValue(":id", id);

    return query.exec();
}

QList<NoteItem> DatabaseManager::getAllNotes()
{
    QList<NoteItem> notes;

    if (!m_initialized) {
        return notes;
    }

    QSqlQuery query(m_db);
    query.exec("SELECT id, title, updated_at FROM notes ORDER BY updated_at DESC");

    while (query.next()) {
        NoteItem item;
        item.id = query.value(0).toInt();
        item.title = query.value(1).toString();
        item.updatedAt = query.value(2).toDateTime();
        notes.append(item);
    }

    return notes;
}

bool DatabaseManager::addNote(const QString &title, const QString &content)
{
    if (!m_initialized) return false;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO notes (title, content) VALUES (:title, :content)");
    query.bindValue(":title", title);
    query.bindValue(":content", content);

    return query.exec();
}

bool DatabaseManager::updateNote(int id, const QString &title, const QString &content)
{
    if (!m_initialized) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE notes SET title = :title, content = :content WHERE id = :id");
    query.bindValue(":title", title);
    query.bindValue(":content", content);
    query.bindValue(":id", id);

    return query.exec();
}


bool DatabaseManager::deleteNote(int id)
{
    if (!m_initialized) return false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM notes WHERE id = :id");
    query.bindValue(":id", id);

    return query.exec();
}


QString DatabaseManager::getNoteContent(int id)
{
    if (!m_initialized) return QString();

    QSqlQuery query(m_db);
    query.prepare("SELECT content FROM notes WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return QString();
}
