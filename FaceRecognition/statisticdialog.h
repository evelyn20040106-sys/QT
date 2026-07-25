/**
 * @file statisticdialog.h
 * @brief 打卡统计对话框头文件，定义考勤数据统计查询界面类
 * @author 人脸识别考勤系统开发团队
 * @date 2026-07-08
 */

#ifndef STATISTICDIALOG_H
#define STATISTICDIALOG_H

#include <QDialog>       /*!< Qt 对话框基类 */
#include <QTableWidget>  /*!< 表格控件，用于展示考勤记录 */
#include <QPushButton>   /*!< 按钮控件 */
#include <QDateEdit>     /*!< 日期选择控件 */
#include <QComboBox>     /*!< 下拉选择框 */
#include <QLabel>        /*!< 文本标签，用于显示统计摘要 */

#include "databasemanager.h"  /*!< 数据库管理器头文件，包含 AttendanceRecord 结构定义 */

class DatabaseManager;   /*!< 前向声明：数据库管理器类 */

/**
 * @class StatisticDialog
 * @brief 考勤统计对话框，提供按日期和按月查询打卡记录的功能
 *
 * 支持两种查询方式：
 * - 按日期查询：查询某一天的所有打卡记录
 * - 按月查询：查询某个月份的所有打卡记录
 *
 * 查询结果以表格展示，并在顶部汇总正常、迟到、早退和异常的记录数量。
 */
class StatisticDialog : public QDialog
{
    Q_OBJECT             /*!< Qt 元对象宏，支持信号槽机制 */

public:
    /**
     * @brief 构造函数
     * @param db     数据库管理器指针，用于查询考勤记录
     * @param parent 父窗口指针，默认为 nullptr
     */
    explicit StatisticDialog(DatabaseManager *db, QWidget *parent = nullptr);

private slots:
    /**
     * @brief 按日期查询槽函数，查询选中日期的所有打卡记录
     *
     * 从日期选择控件中获取日期，调用数据库按日期查询接口，
     * 将结果加载到表格中。
     */
    void onQueryByDate();

    /**
     * @brief 按月查询槽函数，查询选中月份的所有打卡记录
     *
     * 从月份选择控件中获取年月，调用数据库按月查询接口，
     * 将结果加载到表格中。
     */
    void onQueryByMonth();

private:
    /**
     * @brief 初始化用户界面，布局查询条件、摘要标签和记录表格
     */
    void setupUI();

    /**
     * @brief 将查询结果加载到表格中，并更新统计摘要
     * @param records 考勤记录列表，由数据库查询返回
     *
     * 遍历记录列表，逐行插入表格，根据状态设置不同颜色标识：
     * - 正常 → 绿色
     * - 迟到 → 暗黄色
     * - 早退 → 暗红色
     * - 异常 → 红色
     * 同时在摘要标签中显示各类记录的数量统计。
     */
    void loadData(const QList<AttendanceRecord> &records);

    /* ---- 核心业务对象 ---- */
    DatabaseManager *m_db;          /*!< 数据库管理器，提供考勤记录的查询操作 */

    /* ---- 查询控件 ---- */
    QDateEdit *m_dateEdit;          /*!< 日期选择控件，用于按日查询 */
    QDateEdit *m_monthEdit;         /*!< 月份选择控件（显示格式 yyyy-MM），用于按月查询 */
    QPushButton *m_queryDateBtn;    /*!< "按日期查询"按钮 */
    QPushButton *m_queryMonthBtn;   /*!< "按月查询"按钮 */

    /* ---- 展示控件 ---- */
    QTableWidget *m_table;          /*!< 考勤记录表格，列：ID、姓名、部门、日期、上班时间、下班时间、状态 */
    QLabel *m_summaryLabel;         /*!< 统计摘要标签，显示总记录数及各类状态的数量 */
};

#endif // STATISTICDIALOG_H
