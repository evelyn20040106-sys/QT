/**
 * @file employeedialog.h
 * @brief 员工管理对话框头文件，定义员工信息管理界面类
 * @author 人脸识别考勤系统开发团队
 * @date 2026-07-08
 */

#ifndef EMPLOYEEDIALOG_H
#define EMPLOYEEDIALOG_H

#include <QDialog>       /*!< Qt 对话框基类 */
#include <QTableWidget>  /*!< 表格控件，用于展示员工列表 */
#include <QPushButton>   /*!< 按钮控件 */
#include <QLineEdit>     /*!< 单行文本输入框，用于搜索 */

class DatabaseManager;   /*!< 前向声明：数据库管理器类 */

/**
 * @class EmployeeDialog
 * @brief 员工管理对话框，提供员工信息的增、删、改、查功能
 *
 * 以表格形式展示所有员工，支持按姓名搜索、新增员工、编辑员工
 * 信息和删除员工（同时删除该员工的所有打卡记录）。
 */
class EmployeeDialog : public QDialog
{
    Q_OBJECT             /*!< Qt 元对象宏，支持信号槽机制 */

public:
    /**
     * @brief 构造函数
     * @param db     数据库管理器指针，用于操作员工数据
     * @param parent 父窗口指针，默认为 nullptr
     */
    explicit EmployeeDialog(DatabaseManager *db, QWidget *parent = nullptr);

private slots:
    /**
     * @brief 新增员工槽函数，弹出输入对话框录入姓名和部门
     *
     * 验证姓名非空后构造员工对象，调用数据库接口添加，
     * 操作完成后刷新表格数据。
     */
    void onAdd();

    /**
     * @brief 编辑员工槽函数，弹出编辑对话框修改选中员工的姓名和部门
     *
     * 从当前选中行获取员工 ID，加载已有信息供修改，
     * 确认后更新数据库并刷新表格。
     */
    void onEdit();

    /**
     * @brief 删除员工槽函数，确认后删除选中员工及其所有打卡记录
     *
     * 弹出确认对话框，用户确认后调用数据库删除接口，
     * 操作完成后刷新表格数据。
     */
    void onDelete();

    /**
     * @brief 搜索槽函数，根据搜索框中的姓名关键字过滤员工列表
     *
     * 调用 loadData 并传入搜索关键字，实现模糊匹配过滤。
     */
    void onSearch();

private:
    /**
     * @brief 初始化用户界面，布局表格、搜索栏和操作按钮
     */
    void setupUI();

    /**
     * @brief 加载员工数据到表格中
     * @param filter 可选的姓名过滤关键字（大小写不敏感），为空时加载全部员工
     */
    void loadData(const QString &filter = "");

    /* ---- 核心业务对象 ---- */
    DatabaseManager *m_db;          /*!< 数据库管理器，提供员工信息的 CRUD 操作 */

    /* ---- UI 控件 ---- */
    QTableWidget *m_table;          /*!< 员工信息表格，显示 ID、姓名、部门、注册时间 */
    QLineEdit *m_searchEdit;        /*!< 搜索输入框，输入姓名关键字进行过滤 */
    QPushButton *m_addBtn;          /*!< "新增"按钮，用于添加新员工 */
    QPushButton *m_editBtn;         /*!< "编辑"按钮，用于修改选中员工信息 */
    QPushButton *m_deleteBtn;       /*!< "删除"按钮，用于删除选中员工 */
    QPushButton *m_searchBtn;       /*!< "搜索"按钮，触发姓名过滤查询 */
};

#endif // EMPLOYEEDIALOG_H
