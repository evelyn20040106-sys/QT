/**
 * @file databasemanager.h
 * @brief 数据库管理器头文件，定义员工信息、考勤记录数据结构及数据库操作接口
 * @author 作者
 * @date 2026-07-08
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>       /*!< Qt 对象基类 */
#include <QSqlDatabase>  /*!< Qt SQL 数据库类 */
#include <QList>         /*!< Qt 泛型列表容器 */
#include <QDateTime>     /*!< Qt 日期时间类 */

/**
 * @struct EmployeeInfo
 * @brief 员工信息数据结构体，用于存储单个员工的完整信息
 */
struct EmployeeInfo {
    int id = -1;                    /*!< 员工ID，-1 表示未设置 */
    QString name;                   /*!< 员工姓名 */
    QString department;             /*!< 所属部门 */
    QByteArray faceFeature;         /*!< LBPH 人脸特征数据（二进制） */
    QDateTime registeredAt;         /*!< 注册时间 */
};

/**
 * @struct AttendanceRecord
 * @brief 考勤记录数据结构体，用于存储单个员工的某天考勤信息
 */
struct AttendanceRecord {
    int id = -1;                    /*!< 记录ID，-1 表示未设置 */
    int employeeId = -1;            /*!< 员工ID，关联 employees 表 */
    QString employeeName;           /*!< 员工姓名（联合查询结果） */
    QString department;             /*!< 所属部门（联合查询结果） */
    QString date;                   /*!< 考勤日期，格式 yyyy-MM-dd */
    QString clockIn;                /*!< 上班打卡时间，格式 HH:mm */
    QString clockOut;               /*!< 下班打卡时间，格式 HH:mm */
    QString status;                 /*!< 考勤状态："正常"、"迟到"、"早退"、"旷工" */
};

/**
 * @class DatabaseManager
 * @brief 数据库管理器类，继承自 QObject，负责 SQLite 数据库的初始化、员工 CRUD、
 *        人脸特征存取和考勤记录管理等所有数据库操作
 */
class DatabaseManager : public QObject
{
    Q_OBJECT  /*!< Qt 元对象编译器宏，启用信号/槽机制 */

public:
    /**
     * @brief 构造函数
     * @param parent Qt 父对象指针，默认为 nullptr
     */
    explicit DatabaseManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数，负责关闭数据库连接
     */
    ~DatabaseManager();

    /**
     * @brief 初始化数据库：创建/打开 SQLite 数据库文件，并建立所需的数据表
     * @return 成功返回 true，失败返回 false
     */
    bool initialize();

    // ======================== 员工 CRUD 操作 ========================

    /**
     * @brief 添加新员工
     * @param info 员工信息结构体，包含姓名、部门、人脸特征等
     * @return 成功返回 true，失败返回 false
     */
    bool addEmployee(const EmployeeInfo &info);

    /**
     * @brief 更新员工信息
     * @param info 员工信息结构体，需包含有效的 id 字段
     * @return 成功返回 true，失败返回 false
     */
    bool updateEmployee(const EmployeeInfo &info);

    /**
     * @brief 删除指定员工（同时删除其所有考勤记录）
     * @param id 员工ID
     * @return 成功返回 true，失败返回 false
     */
    bool deleteEmployee(int id);

    /**
     * @brief 根据 ID 获取单个员工信息
     * @param id 员工ID
     * @return 员工信息结构体（未找到时返回 id=-1 的空结构体）
     */
    EmployeeInfo getEmployee(int id);

    /**
     * @brief 获取所有员工列表
     * @return 包含所有员工的列表，按 ID 升序排列
     */
    QList<EmployeeInfo> getAllEmployees();

    // ======================== 人脸特征操作 ========================

    /**
     * @brief 保存/更新指定员工的人脸特征数据
     * @param employeeId 员工ID
     * @param feature 人脸特征二进制数据
     * @return 成功返回 true，失败返回 false
     */
    bool saveFaceFeature(int employeeId, const QByteArray &feature);

    /**
     * @brief 获取指定员工的人脸特征数据
     * @param employeeId 员工ID
     * @return 人脸特征二进制数据（未找到时返回空 QByteArray）
     */
    QByteArray getFaceFeature(int employeeId);

    // ======================== 考勤操作 ========================

    /**
     * @brief 上班打卡，9:00 之前打卡状态为"正常"，之后为"迟到"
     * @param employeeId 员工ID
     * @return 打卡成功返回 true；已存在打卡记录返回 false
     */
    bool clockIn(int employeeId);

    /**
     * @brief 下班打卡，根据上下班时间自动判断考勤状态
     * @param employeeId 员工ID
     * @return 成功返回 true，失败返回 false
     */
    bool clockOut(int employeeId);

    /**
     * @brief 获取指定员工今天的考勤记录
     * @param employeeId 员工ID
     * @return 考勤记录结构体（未找到时返回 id=-1 的空结构体）
     */
    AttendanceRecord getTodayRecord(int employeeId);

    /**
     * @brief 获取指定日期的所有考勤记录
     * @param date 日期字符串，格式 yyyy-MM-dd
     * @return 该日期的考勤记录列表，按打卡时间排序
     */
    QList<AttendanceRecord> getRecordsByDate(const QString &date);

    /**
     * @brief 获取指定员工的所有考勤记录
     * @param employeeId 员工ID
     * @return 该员工的考勤记录列表，按日期降序排列
     */
    QList<AttendanceRecord> getRecordsByEmployee(int employeeId);

    /**
     * @brief 获取指定月份的所有考勤记录
     * @param yearMonth 年月字符串，格式 yyyy-MM
     * @return 该月份的考勤记录列表，按日期和员工 ID 排序
     */
    QList<AttendanceRecord> getRecordsByMonth(const QString &yearMonth);

    /**
     * @brief 获取所有考勤记录
     * @return 全部考勤记录列表，按日期降序、员工 ID 排序
     */
    QList<AttendanceRecord> getAllRecords();

private:
    /**
     * @brief 创建 employees 和 attendance 两张数据表（如果尚未存在）
     * @return 成功返回 true，失败返回 false
     */
    bool createTables();

    /**
     * @brief 获取今天的日期字符串
     * @return 格式为 yyyy-MM-dd 的日期字符串
     */
    QString getTodayDate() const;

    /**
     * @brief 获取当前时间字符串
     * @return 格式为 HH:mm 的时间字符串
     */
    QString getCurrentTime() const;

    QSqlDatabase m_db;  /*!< SQLite 数据库连接对象 */
};

#endif // DATABASEMANAGER_H
