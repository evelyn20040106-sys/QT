/**
 * @file databasemanager.cpp
 * @brief 数据库管理器实现文件，实现 SQLite 数据库的所有读写操作
 * @author 作者
 * @date 2026-07-08
 */

#include "databasemanager.h"
#include <QSqlQuery>         /*!< SQL 查询执行类 */
#include <QSqlError>         /*!< SQL 数据库错误信息类 */
#include <QDebug>            /*!< 调试输出工具 */
#include <QStandardPaths>    /*!< 标准路径查询工具 */
#include <QDir>              /*!< 目录操作工具 */
#include <QCoreApplication>  /*!< Qt 核心应用类，用于获取应用路径 */

/**
 * @brief 构造函数，初始化父对象
 * @param parent Qt 父对象指针，默认为 nullptr
 */
DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)  /*!< 将 parent 传递给 QObject 基类构造 */
{
}

/**
 * @brief 析构函数，释放数据库连接
 */
DatabaseManager::~DatabaseManager()
{
    /* 判断数据库是否处于打开状态 */
    if (m_db.isOpen()) {
        m_db.close();  /*!< 关闭数据库连接，释放资源 */
    }
}

/**
 * @brief 初始化数据库：创建 SQLite 数据库连接、设置数据库文件路径、打开数据库并创建表
 * @return 成功返回 true，失败返回 false
 */
bool DatabaseManager::initialize()
{
    /* 添加 SQLite 数据库驱动，获取默认连接 */
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    /* 构造数据库文件路径：应用当前目录下的 attendance.db */
    QString dbPath = QCoreApplication::applicationDirPath() + "/attendance.db";
    m_db.setDatabaseName(dbPath);  /*!< 设置数据库文件名 */

    /* 尝试打开数据库 */
    if (!m_db.open()) {
        /* 打开失败，输出错误日志并返回 false */
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    /* 数据库打开成功，创建所需的数据表 */
    return createTables();
}

/**
 * @brief（私有）创建 employees（员工表）和 attendance（考勤表），使用 IF NOT EXISTS 避免重复创建
 * @return 两张表都创建成功返回 true，任一失败返回 false
 */
bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);  /*!< 创建基于当前数据库连接的查询对象 */

    /**
     * 创建 employees 表
     * 字段：id(自增主键), name(姓名), department(部门),
     *       face_feature(人脸特征BLOB), registered_at(注册时间)
     */
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS employees ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  department TEXT DEFAULT '',"
        "  face_feature BLOB,"
        "  registered_at TEXT"
        ")"
    );
    if (!ok) {
        /* 创建 employees 表失败，输出错误日志 */
        qCritical() << "Failed to create employees table:" << query.lastError().text();
        return false;
    }

    /**
     * 创建 attendance 表
     * 字段：id(自增主键), employee_id(员工ID外键), date(日期),
     *       clock_in(上班时间), clock_out(下班时间), status(考勤状态)
     */
    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS attendance ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  employee_id INTEGER NOT NULL,"
        "  date TEXT NOT NULL,"
        "  clock_in TEXT,"
        "  clock_out TEXT,"
        "  status TEXT DEFAULT '正常',"
        "  FOREIGN KEY (employee_id) REFERENCES employees(id)"
        ")"
    );
    if (!ok) {
        /* 创建 attendance 表失败，输出错误日志 */
        qCritical() << "Failed to create attendance table:" << query.lastError().text();
        return false;
    }

    /* 两张表均创建成功 */
    return true;
}

// ======================== 员工 CRUD 操作 ========================

/**
 * @brief 添加新员工记录到 employees 表
 * @param info 员工信息结构体，包含姓名、部门、人脸特征
 * @return 成功返回 true，失败返回 false
 */
bool DatabaseManager::addEmployee(const EmployeeInfo &info)
{
    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /* 使用参数化查询插入员工记录，防止 SQL 注入 */
    query.prepare(
        "INSERT INTO employees (name, department, face_feature, registered_at) "
        "VALUES (:name, :dept, :feature, :time)"
    );
    query.bindValue(":name", info.name);                    /*!< 绑定员工姓名 */
    query.bindValue(":dept", info.department);              /*!< 绑定部门 */
    query.bindValue(":feature", info.faceFeature);          /*!< 绑定人脸特征数据 */
    /* 注册时间取当前系统时间，格式为 ISO 标准日期时间字符串 */
    query.bindValue(":time", QDateTime::currentDateTime().toString(Qt::ISODate));

    /* 执行插入操作 */
    if (!query.exec()) {
        /* 插入失败，输出警告日志 */
        qWarning() << "addEmployee failed:" << query.lastError().text();
        return false;
    }
    return true;
}

/**
 * @brief 更新指定员工的信息（姓名、部门、人脸特征）
 * @param info 员工信息结构体，需包含有效的 id 字段
 * @return 成功返回 true，失败返回 false
 */
bool DatabaseManager::updateEmployee(const EmployeeInfo &info)
{
    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /* 使用参数化查询更新员工记录 */
    query.prepare(
        "UPDATE employees SET name=:name, department=:dept, face_feature=:feature "
        "WHERE id=:id"
    );
    query.bindValue(":id", info.id);                        /*!< 绑定员工ID */
    query.bindValue(":name", info.name);                    /*!< 绑定新姓名 */
    query.bindValue(":dept", info.department);              /*!< 绑定新部门 */
    query.bindValue(":feature", info.faceFeature);          /*!< 绑定新人脸特征 */

    /* 执行更新操作 */
    if (!query.exec()) {
        /* 更新失败，输出警告日志 */
        qWarning() << "updateEmployee failed:" << query.lastError().text();
        return false;
    }
    return true;
}

/**
 * @brief 删除指定员工及其所有考勤记录
 * @param id 员工ID
 * @return 成功返回 true，失败返回 false
 */
bool DatabaseManager::deleteEmployee(int id)
{
    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /* 第一步：先删除该员工的所有考勤记录，维护外键约束 */
    query.prepare("DELETE FROM attendance WHERE employee_id=:id");
    query.bindValue(":id", id);
    query.exec();  /*!< 考勤删除失败不影响主删除流程 */

    /* 第二步：删除员工记录 */
    query.prepare("DELETE FROM employees WHERE id=:id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        /* 删除员工失败，输出警告日志 */
        qWarning() << "deleteEmployee failed:" << query.lastError().text();
        return false;
    }
    return true;
}

/**
 * @brief 根据 ID 查询员工信息
 * @param id 员工ID
 * @return 员工信息结构体；若未找到则返回 id=-1 的空结构体
 */
EmployeeInfo DatabaseManager::getEmployee(int id)
{
    EmployeeInfo info;       /*!< 创建空的员工信息结构体（默认 id=-1） */
    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /* 参数化查询：根据 ID 查询员工所有字段 */
    query.prepare("SELECT * FROM employees WHERE id=:id");
    query.bindValue(":id", id);

    /* 执行查询并移动到第一条结果 */
    if (query.exec() && query.next()) {
        /* 逐字段填充结构体 */
        info.id = query.value("id").toInt();                    /*!< 员工ID */
        info.name = query.value("name").toString();            /*!< 姓名 */
        info.department = query.value("department").toString(); /*!< 部门 */
        info.faceFeature = query.value("face_feature").toByteArray();  /*!< 人脸特征 */
        /* 注册时间字符串转 QDateTime */
        info.registeredAt = QDateTime::fromString(query.value("registered_at").toString(), Qt::ISODate);
    }
    return info;  /*!< 返回查询结果（未查到则返回空结构体） */
}

/**
 * @brief 获取所有员工列表
 * @return 所有员工按 ID 升序排列的列表
 */
QList<EmployeeInfo> DatabaseManager::getAllEmployees()
{
    QList<EmployeeInfo> list;  /*!< 用于存放查询结果的列表 */
    QSqlQuery query(m_db);     /*!< 创建查询对象 */

    /* 查询 employees 表全部记录，按 id 升序排列 */
    if (query.exec("SELECT * FROM employees ORDER BY id")) {
        /* 逐行遍历查询结果 */
        while (query.next()) {
            EmployeeInfo info;  /*!< 临时结构体 */
            /* 读取当前行各字段 */
            info.id = query.value("id").toInt();
            info.name = query.value("name").toString();
            info.department = query.value("department").toString();
            info.faceFeature = query.value("face_feature").toByteArray();
            info.registeredAt = QDateTime::fromString(query.value("registered_at").toString(), Qt::ISODate);
            list.append(info);  /*!< 添加到列表尾部 */
        }
    }
    return list;  /*!< 返回员工列表（可能为空列表） */
}

// ======================== 人脸特征操作 ========================

/**
 * @brief 保存或更新指定员工的人脸特征数据
 * @param employeeId 员工ID
 * @param feature 人脸特征二进制数据（LBPH 算法提取）
 * @return 成功返回 true，失败返回 false
 */
bool DatabaseManager::saveFaceFeature(int employeeId, const QByteArray &feature)
{
    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /* 更新指定员工的人脸特征字段 */
    query.prepare("UPDATE employees SET face_feature=:feature WHERE id=:id");
    query.bindValue(":id", employeeId);      /*!< 绑定员工ID */
    query.bindValue(":feature", feature);    /*!< 绑定人脸特征数据 */

    /* 直接返回执行结果 */
    return query.exec();
}

/**
 * @brief 获取指定员工的人脸特征数据
 * @param employeeId 员工ID
 * @return 人脸特征二进制数据；未找到时返回空 QByteArray
 */
QByteArray DatabaseManager::getFaceFeature(int employeeId)
{
    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /* 查询指定员工的人脸特征字段 */
    query.prepare("SELECT face_feature FROM employees WHERE id=:id");
    query.bindValue(":id", employeeId);

    /* 执行查询并检查是否有结果 */
    if (query.exec() && query.next()) {
        /* 返回人脸特征二进制数据 */
        return query.value("face_feature").toByteArray();
    }
    return {};  /*!< 未找到记录，返回空 QByteArray */
}

// ======================== 考勤操作 ========================

/**
 * @brief 上班打卡：若当前时间 > 09:00 则状态标记为"迟到"，否则为"正常"
 * @details 如果今天已经打过卡则不再重复记录
 * @param employeeId 员工ID
 * @return 打卡成功返回 true；已存在打卡记录返回 false
 */
bool DatabaseManager::clockIn(int employeeId)
{
    /* 获取今天的日期和当前时间 */
    QString today = getTodayDate();   /*!< 格式: yyyy-MM-dd */
    QString now = getCurrentTime();   /*!< 格式: HH:mm */

    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /**
     * 使用 INSERT OR REPLACE 插入/替换考勤记录
     * 根据打卡时间自动判断状态：
     *   time > '09:00' → "迟到"
     *   否则          → "正常"
     */
    query.prepare(
        "INSERT OR REPLACE INTO attendance (employee_id, date, clock_in, status) "
        "VALUES (:eid, :date, :time, "
        "  CASE WHEN :time > '09:00' THEN '迟到' ELSE '正常' END)"
    );
    query.bindValue(":eid", employeeId);   /*!< 绑定员工ID */
    query.bindValue(":date", today);       /*!< 绑定今日日期 */
    query.bindValue(":time", now);         /*!< 绑定当前时间 */

    /* 检查今天是否已经打过卡 */
    AttendanceRecord existing = getTodayRecord(employeeId);
    if (existing.id > 0) {
        /* 已存在打卡记录，不覆盖，返回 false */
        return false;
    }

    /* 执行插入操作 */
    if (!query.exec()) {
        /* INSERT 失败，输出警告日志 */
        qWarning() << "clockIn failed:" << query.lastError().text();
        /* 尝试使用标准 INSERT（可能 REPLACE 因某些原因失败） */
        query.prepare(
            "INSERT INTO attendance (employee_id, date, clock_in, status) "
            "VALUES (:eid, :date, :time, "
            "  CASE WHEN :time > '09:00' THEN '迟到' ELSE '正常' END)"
        );
        query.bindValue(":eid", employeeId);
        query.bindValue(":date", today);
        query.bindValue(":time", now);
        if (!query.exec()) {
            /* 再次失败，返回 false */
            return false;
        }
    }
    return true;
}

/**
 * @brief 下班打卡：根据上下班时间组合自动判断考勤状态
 * @details 若没有上班打卡记录则创建一条仅含下班时间的异常记录
 * @param employeeId 员工ID
 * @return 成功返回 true，失败返回 false
 */
bool DatabaseManager::clockOut(int employeeId)
{
    /* 获取今天的日期和当前时间 */
    QString today = getTodayDate();   /*!< 格式: yyyy-MM-dd */
    QString now = getCurrentTime();   /*!< 格式: HH:mm */

    QSqlQuery query(m_db);  /*!< 创建查询对象 */

    /* 先查询今天的考勤记录 */
    AttendanceRecord record = getTodayRecord(employeeId);
    if (record.id < 0 || record.clockIn.isEmpty()) {
        /**
         * 无打卡记录或只有下班没有上班记录：
         * 创建一条仅包含下班时间的新记录，状态标记为"异常"
         */
        query.prepare(
            "INSERT INTO attendance (employee_id, date, clock_out, status) "
            "VALUES (:eid, :date, :time, '异常')"
        );
        query.bindValue(":eid", employeeId);   /*!< 绑定员工ID */
        query.bindValue(":date", today);       /*!< 绑定今日日期 */
        query.bindValue(":time", now);         /*!< 绑定当前时间 */
    } else {
        /**
         * 已有上班打卡记录，更新下班时间并自动判断最终考勤状态：
         *   time < '18:00' 且上班不迟到              → "早退"
         *   status='迟到'  且 time >= '18:00'       → "迟到"
         *   status='迟到'  且 time < '18:00'        → "迟到早退"
         *   其他                                     → "正常"
         */
        query.prepare(
            "UPDATE attendance SET clock_out=:time, "
            "  status=CASE "
            "    WHEN :time < '18:00' AND clock_in <= '09:00' THEN '早退' "
            "    WHEN status='迟到' AND :time >= '18:00' THEN '迟到' "
            "    WHEN status='迟到' AND :time < '18:00' THEN '迟到早退' "
            "    ELSE '正常' "
            "  END "
            "WHERE employee_id=:eid AND date=:date"
        );
        query.bindValue(":eid", employeeId);   /*!< 绑定员工ID */
        query.bindValue(":date", today);       /*!< 绑定今日日期 */
        query.bindValue(":time", now);         /*!< 绑定当前时间 */
    }

    /* 执行插入或更新操作 */
    if (!query.exec()) {
        /* 操作失败，输出警告日志 */
        qWarning() << "clockOut failed:" << query.lastError().text();
        return false;
    }
    return true;
}

/**
 * @brief 获取指定员工今天的考勤记录（联合查询员工姓名和部门）
 * @param employeeId 员工ID
 * @return 考勤记录结构体；未找到时返回 id=-1 的空结构体
 */
AttendanceRecord DatabaseManager::getTodayRecord(int employeeId)
{
    AttendanceRecord record;  /*!< 默认构造 id=-1 的空记录 */
    QSqlQuery query(m_db);    /*!< 创建查询对象 */

    /**
     * 联合查询 attendance 表和 employees 表
     * 获取考勤记录的同时获取员工姓名和部门信息
     */
    query.prepare(
        "SELECT a.*, e.name, e.department FROM attendance a "
        "LEFT JOIN employees e ON a.employee_id=e.id "
        "WHERE a.employee_id=:eid AND a.date=:date"
    );
    query.bindValue(":eid", employeeId);        /*!< 绑定员工ID */
    query.bindValue(":date", getTodayDate());   /*!< 绑定今天日期 */

    /* 执行查询并读取第一条结果 */
    if (query.exec() && query.next()) {
        /* 填充考勤记录结构体 */
        record.id = query.value("id").toInt();                    /*!< 记录ID */
        record.employeeId = query.value("employee_id").toInt();  /*!< 员工ID */
        record.employeeName = query.value("name").toString();    /*!< 员工姓名 */
        record.department = query.value("department").toString(); /*!< 部门 */
        record.date = query.value("date").toString();            /*!< 日期 */
        record.clockIn = query.value("clock_in").toString();     /*!< 上班时间 */
        record.clockOut = query.value("clock_out").toString();   /*!< 下班时间 */
        record.status = query.value("status").toString();        /*!< 考勤状态 */
    }
    return record;  /*!< 返回记录（未找到则为空记录） */
}

/**
 * @brief 获取指定日期的所有考勤记录
 * @param date 日期字符串，格式 yyyy-MM-dd
 * @return 该日期的考勤记录列表，按打卡时间升序排列
 */
QList<AttendanceRecord> DatabaseManager::getRecordsByDate(const QString &date)
{
    QList<AttendanceRecord> list;  /*!< 用于存放查询结果的列表 */
    QSqlQuery query(m_db);         /*!< 创建查询对象 */

    /* 联合查询指定日期的考勤记录，按打卡时间排序 */
    query.prepare(
        "SELECT a.*, e.name, e.department FROM attendance a "
        "LEFT JOIN employees e ON a.employee_id=e.id "
        "WHERE a.date=:date ORDER BY a.clock_in"
    );
    query.bindValue(":date", date);  /*!< 绑定查询日期 */

    if (query.exec()) {
        /* 逐行遍历查询结果 */
        while (query.next()) {
            AttendanceRecord r;  /*!< 临时考勤记录 */
            /* 读取当前行各字段 */
            r.id = query.value("id").toInt();
            r.employeeId = query.value("employee_id").toInt();
            r.employeeName = query.value("name").toString();
            r.department = query.value("department").toString();
            r.date = query.value("date").toString();
            r.clockIn = query.value("clock_in").toString();
            r.clockOut = query.value("clock_out").toString();
            r.status = query.value("status").toString();
            list.append(r);  /*!< 添加到列表尾部 */
        }
    }
    return list;  /*!< 返回考勤记录列表 */
}

/**
 * @brief 获取指定员工的所有考勤记录
 * @param employeeId 员工ID
 * @return 该员工的所有考勤记录列表，按日期降序排列（最新的在前）
 */
QList<AttendanceRecord> DatabaseManager::getRecordsByEmployee(int employeeId)
{
    QList<AttendanceRecord> list;  /*!< 用于存放查询结果的列表 */
    QSqlQuery query(m_db);         /*!< 创建查询对象 */

    /* 联合查询指定员工的考勤记录，按日期降序排列 */
    query.prepare(
        "SELECT a.*, e.name, e.department FROM attendance a "
        "LEFT JOIN employees e ON a.employee_id=e.id "
        "WHERE a.employee_id=:eid ORDER BY a.date DESC"
    );
    query.bindValue(":eid", employeeId);  /*!< 绑定员工ID */

    if (query.exec()) {
        /* 逐行遍历查询结果 */
        while (query.next()) {
            AttendanceRecord r;  /*!< 临时考勤记录 */
            /* 读取当前行各字段 */
            r.id = query.value("id").toInt();
            r.employeeId = query.value("employee_id").toInt();
            r.employeeName = query.value("name").toString();
            r.department = query.value("department").toString();
            r.date = query.value("date").toString();
            r.clockIn = query.value("clock_in").toString();
            r.clockOut = query.value("clock_out").toString();
            r.status = query.value("status").toString();
            list.append(r);  /*!< 添加到列表尾部 */
        }
    }
    return list;  /*!< 返回考勤记录列表 */
}

/**
 * @brief 获取指定月份的所有考勤记录
 * @param yearMonth 年月字符串，格式 yyyy-MM（如 "2026-07"）
 * @return 该月份的考勤记录列表，按日期和员工ID排序
 */
QList<AttendanceRecord> DatabaseManager::getRecordsByMonth(const QString &yearMonth)
{
    QList<AttendanceRecord> list;  /*!< 用于存放查询结果的列表 */
    QSqlQuery query(m_db);         /*!< 创建查询对象 */

    /* 使用 LIKE 模糊匹配该月份的所有记录，按日期和员工排序 */
    query.prepare(
        "SELECT a.*, e.name, e.department FROM attendance a "
        "LEFT JOIN employees e ON a.employee_id=e.id "
        "WHERE a.date LIKE :pattern ORDER BY a.date, a.employee_id"
    );
    /* 拼接 LIKE 模式：例如 "2026-07%" 匹配 7 月所有日期 */
    query.bindValue(":pattern", yearMonth + "%");

    if (query.exec()) {
        /* 逐行遍历查询结果 */
        while (query.next()) {
            AttendanceRecord r;  /*!< 临时考勤记录 */
            /* 读取当前行各字段 */
            r.id = query.value("id").toInt();
            r.employeeId = query.value("employee_id").toInt();
            r.employeeName = query.value("name").toString();
            r.department = query.value("department").toString();
            r.date = query.value("date").toString();
            r.clockIn = query.value("clock_in").toString();
            r.clockOut = query.value("clock_out").toString();
            r.status = query.value("status").toString();
            list.append(r);  /*!< 添加到列表尾部 */
        }
    }
    return list;  /*!< 返回考勤记录列表 */
}

/**
 * @brief 获取所有考勤记录
 * @return 全部考勤记录列表，按日期降序、员工ID排序
 */
QList<AttendanceRecord> DatabaseManager::getAllRecords()
{
    QList<AttendanceRecord> list;  /*!< 用于存放查询结果的列表 */
    QSqlQuery query(m_db);         /*!< 创建查询对象 */

    /* 联合查询全部考勤记录，按日期降序、员工ID排序 */
    if (query.exec(
            "SELECT a.*, e.name, e.department FROM attendance a "
            "LEFT JOIN employees e ON a.employee_id=e.id "
            "ORDER BY a.date DESC, a.employee_id")) {
        /* 逐行遍历查询结果 */
        while (query.next()) {
            AttendanceRecord r;  /*!< 临时考勤记录 */
            /* 读取当前行各字段 */
            r.id = query.value("id").toInt();
            r.employeeId = query.value("employee_id").toInt();
            r.employeeName = query.value("name").toString();
            r.department = query.value("department").toString();
            r.date = query.value("date").toString();
            r.clockIn = query.value("clock_in").toString();
            r.clockOut = query.value("clock_out").toString();
            r.status = query.value("status").toString();
            list.append(r);  /*!< 添加到列表尾部 */
        }
    }
    return list;  /*!< 返回考勤记录列表 */
}

/**
 * @brief（私有）获取今天的日期字符串
 * @return 当前系统日期的 yyyy-MM-dd 格式字符串
 */
QString DatabaseManager::getTodayDate() const
{
    return QDate::currentDate().toString("yyyy-MM-dd");
}

/**
 * @brief（私有）获取当前时间字符串
 * @return 当前系统时间的 HH:mm 格式字符串
 */
QString DatabaseManager::getCurrentTime() const
{
    return QTime::currentTime().toString("HH:mm");
}
