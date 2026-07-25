/**
 * @file employeedialog.cpp
 * @brief 员工管理对话框实现文件，实现员工信息的增删改查功能
 * @author 人脸识别考勤系统开发团队
 * @date 2026-07-08
 */

#include "employeedialog.h"  /*!< 员工管理对话框头文件 */
#include "databasemanager.h" /*!< 数据库管理器，提供员工数据的 CRUD 操作 */

#include <QVBoxLayout>   /*!< 垂直布局 */
#include <QHBoxLayout>   /*!< 水平布局 */
#include <QFormLayout>   /*!< 表单布局（标签-字段对） */
#include <QDialog>       /*!< 对话框基类，用于新增/编辑弹出窗口 */
#include <QHeaderView>   /*!< 表格头视图，用于设置列宽策略 */
#include <QMessageBox>   /*!< 消息框，用于显示提示/警告/确认信息 */
#include <QInputDialog>  /*!< 输入对话框 */

/**
 * @brief 构造函数：初始化数据库引用，设置 UI 并加载员工数据
 * @param db     数据库管理器指针
 * @param parent 父窗口指针
 */
EmployeeDialog::EmployeeDialog(DatabaseManager *db, QWidget *parent)
    : QDialog(parent)  /*!< 调用基类构造函数 */
    , m_db(db)         /*!< 保存数据库管理器指针 */
{
    setupUI();   /*!< 初始化界面控件布局 */
    loadData();  /*!< 加载所有员工数据到表格 */
}

/**
 * @brief 初始化用户界面：创建搜索栏、员工表格和操作按钮
 *
 * 界面布局从上到下依次为：
 * 1. 搜索栏（搜索输入框 + 搜索按钮）
 * 2. 员工信息表格（ID、姓名、部门、注册时间）
 * 3. 操作按钮行（新增、编辑、删除、关闭）
 */
void EmployeeDialog::setupUI()
{
    /* 设置窗口标题和最小尺寸 */
    setWindowTitle("员工管理");
    setMinimumSize(700, 500);

    /* 创建主垂直布局 */
    auto *mainLayout = new QVBoxLayout(this);

    /* ---- 搜索栏 ---- */
    auto *searchLayout = new QHBoxLayout;                   /*!< 水平搜索布局 */
    m_searchEdit = new QLineEdit;                           /*!< 搜索输入框 */
    m_searchEdit->setPlaceholderText("搜索姓名...");        /*!< 占位提示文字 */
    m_searchBtn = new QPushButton("搜索");                  /*!< 搜索按钮 */
    searchLayout->addWidget(m_searchEdit, 1);               /*!< 输入框占据剩余空间（拉伸因子 1） */
    searchLayout->addWidget(m_searchBtn);                   /*!< 搜索按钮在右侧 */

    /* ---- 员工信息表格 ---- */
    m_table = new QTableWidget(this);                       /*!< 创建表格控件 */
    m_table->setColumnCount(4);                             /*!< 设置 4 列：ID、姓名、部门、注册时间 */
    m_table->setHorizontalHeaderLabels({"ID", "姓名", "部门", "注册时间"}); /*!< 设置表头文字 */
    m_table->horizontalHeader()->setStretchLastSection(true); /*!< 最后一列自动拉伸填满剩余宽度 */
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows); /*!< 点击选中整行 */
    m_table->setSelectionMode(QAbstractItemView::SingleSelection); /*!< 单选模式，一次只能选中一行 */
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   /*!< 禁止直接编辑表格内容 */
    m_table->setAlternatingRowColors(true);                 /*!< 交替行背景色，提高可读性 */

    /* ---- 操作按钮行 ---- */
    auto *btnLayout = new QHBoxLayout;                      /*!< 水平按钮布局 */
    m_addBtn = new QPushButton("新增");                     /*!< 新增按钮 */
    m_editBtn = new QPushButton("编辑");                    /*!< 编辑按钮 */
    m_deleteBtn = new QPushButton("删除");                  /*!< 删除按钮 */
    QPushButton *closeBtn = new QPushButton("关闭");        /*!< 关闭按钮（局部变量，无需成员变量） */

    /* 按钮样式设置 */
    m_addBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; "
                             "padding: 8px 16px; border-radius: 4px; }");       /*!< 绿色：新增 */
    m_editBtn->setStyleSheet("QPushButton { background-color: #FF9800; color: white; "
                              "padding: 8px 16px; border-radius: 4px; }");      /*!< 橙色：编辑 */
    m_deleteBtn->setStyleSheet("QPushButton { background-color: #f44336; color: white; "
                                "padding: 8px 16px; border-radius: 4px; }");    /*!< 红色：删除 */

    /* 按钮添加到水平布局 */
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();                                /*!< 弹性空间，将关闭按钮推到右侧 */
    btnLayout->addWidget(closeBtn);

    /* 将各组件依次加入主布局 */
    mainLayout->addLayout(searchLayout);                    /*!< 搜索栏 */
    mainLayout->addWidget(m_table);                         /*!< 员工表格 */
    mainLayout->addLayout(btnLayout);                       /*!< 按钮行 */

    /* ---- 连接信号槽 ---- */
    connect(m_addBtn, &QPushButton::clicked, this, &EmployeeDialog::onAdd);
    connect(m_editBtn, &QPushButton::clicked, this, &EmployeeDialog::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &EmployeeDialog::onDelete);
    connect(m_searchBtn, &QPushButton::clicked, this, &EmployeeDialog::onSearch);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
}

/**
 * @brief 加载员工数据到表格中
 * @param filter 可选的姓名过滤关键字（大小写不敏感），默认为空字符串表示加载全部
 *
 * 从数据库获取所有员工，遍历并根据过滤条件筛选匹配项，
 * 逐行插入表格（ID、姓名、部门、注册时间）。
 */
void EmployeeDialog::loadData(const QString &filter)
{
    m_table->setRowCount(0);                                /*!< 清空表格所有行，准备重新加载 */
    auto employees = m_db->getAllEmployees();                /*!< 从数据库获取所有员工 */

    int row = 0;                                            /*!< 当前插入行号 */
    for (const auto &emp : employees) {                     /*!< 遍历所有员工 */
        /* 如果有关键字过滤，跳过不匹配的员工（大小写不敏感的模糊匹配） */
        if (!filter.isEmpty() && !emp.name.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        /* 在表格中插入新行并设置各列数据 */
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(emp.id)));            /*!< ID */
        m_table->setItem(row, 1, new QTableWidgetItem(emp.name));                            /*!< 姓名 */
        m_table->setItem(row, 2, new QTableWidgetItem(emp.department));                      /*!< 部门 */
        m_table->setItem(row, 3, new QTableWidgetItem(
            emp.registeredAt.toString("yyyy-MM-dd HH:mm")));                                 /*!< 注册时间 */
        row++;                                               /*!< 行号递增 */
    }
}

/**
 * @brief 新增员工槽函数：弹出表单对话框，录入姓名和部门后保存
 *
 * 使用 QDialog 创建模态表单，包含姓名和部门输入框。
 * 用户点击确定后验证姓名非空，构造 EmployeeInfo 对象并调用数据库添加接口。
 * 操作成功后刷新表格数据。
 */
void EmployeeDialog::onAdd()
{
    /* 创建模态新增对话框 */
    QDialog dlg(this);
    dlg.setWindowTitle("新增员工");
    auto *layout = new QFormLayout(&dlg);                   /*!< 表单布局 */

    /* 创建姓名和部门输入框 */
    auto *nameEdit = new QLineEdit;                         /*!< 姓名输入框 */
    auto *deptEdit = new QLineEdit;                         /*!< 部门输入框 */
    layout->addRow("姓名:", nameEdit);                       /*!< 添加姓名行 */
    layout->addRow("部门:", deptEdit);                       /*!< 添加部门行 */

    /* 确定/取消按钮 */
    auto *btnLayout = new QHBoxLayout;
    auto *okBtn = new QPushButton("确定");
    auto *cancelBtn = new QPushButton("取消");
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addRow(btnLayout);                              /*!< 按钮行添加到表单 */

    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    /* 如果用户点击了"确定" */
    if (dlg.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();          /*!< 去除首尾空格 */
        if (name.isEmpty()) {                               /*!< 姓名为空则报错 */
            QMessageBox::warning(this, "提示", "姓名不能为空");
            return;
        }

        /* 构造员工信息并保存 */
        EmployeeInfo info;
        info.name = name;                                   /*!< 员工姓名 */
        info.department = deptEdit->text().trimmed();       /*!< 员工部门 */
        if (m_db->addEmployee(info)) {                      /*!< 调用数据库添加接口 */
            loadData(m_searchEdit->text());                 /*!< 刷新表格，保持当前搜索过滤 */
        } else {
            QMessageBox::critical(this, "错误", "添加失败");
        }
    }
}

/**
 * @brief 编辑员工槽函数：弹出表单对话框修改选中员工的姓名和部门
 *
 * 从当前选中行获取员工 ID，加载已有信息到编辑表单。
 * 用户修改确认后调用数据库更新接口，刷新表格数据。
 * 如果未选中任何行，提示用户先选择。
 */
void EmployeeDialog::onEdit()
{
    /* 检查是否有选中行 */
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择要编辑的员工");
        return;
    }

    /* 从表格中获取选中行的员工 ID，并从数据库加载完整信息 */
    int id = m_table->item(row, 0)->text().toInt();         /*!< 获取 ID */
    EmployeeInfo info = m_db->getEmployee(id);               /*!< 从数据库加载员工信息 */

    /* 创建模态编辑对话框，预填当前值 */
    QDialog dlg(this);
    dlg.setWindowTitle("编辑员工");
    auto *layout = new QFormLayout(&dlg);

    auto *nameEdit = new QLineEdit(info.name);               /*!< 预填当前姓名 */
    auto *deptEdit = new QLineEdit(info.department);         /*!< 预填当前部门 */
    layout->addRow("姓名:", nameEdit);
    layout->addRow("部门:", deptEdit);

    /* 确定/取消按钮 */
    auto *btnLayout = new QHBoxLayout;
    auto *okBtn = new QPushButton("确定");
    auto *cancelBtn = new QPushButton("取消");
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addRow(btnLayout);

    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    /* 如果用户点击了"确定" */
    if (dlg.exec() == QDialog::Accepted) {
        info.name = nameEdit->text().trimmed();              /*!< 更新姓名 */
        info.department = deptEdit->text().trimmed();        /*!< 更新部门 */
        if (m_db->updateEmployee(info)) {                    /*!< 调用数据库更新接口 */
            loadData(m_searchEdit->text());                  /*!< 刷新表格，保持当前搜索过滤 */
        } else {
            QMessageBox::critical(this, "错误", "更新失败");
        }
    }
}

/**
 * @brief 删除员工槽函数：确认后删除选中员工及其所有打卡记录
 *
 * 从当前选中行获取员工 ID 和姓名，弹出确认对话框告知用户
 * 将同时删除该员工的所有打卡记录。确认后调用数据库删除接口，
 * 操作成功后刷新表格数据。
 */
void EmployeeDialog::onDelete()
{
    /* 检查是否有选中行 */
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择要删除的员工");
        return;
    }

    /* 从表格中获取员工 ID 和姓名 */
    int id = m_table->item(row, 0)->text().toInt();          /*!< 员工 ID */
    QString name = m_table->item(row, 1)->text();            /*!< 员工姓名 */

    /* 弹出确认对话框，提示同时删除打卡记录 */
    auto ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除员工 \"%1\" 吗？\n同时会删除该员工的所有打卡记录。").arg(name),
        QMessageBox::Yes | QMessageBox::No);

    /* 用户确认后执行删除 */
    if (ret == QMessageBox::Yes) {
        if (m_db->deleteEmployee(id)) {                      /*!< 调用数据库删除接口 */
            loadData(m_searchEdit->text());                  /*!< 刷新表格 */
        } else {
            QMessageBox::critical(this, "错误", "删除失败");
        }
    }
}

/**
 * @brief 搜索槽函数：根据搜索框中的姓名关键字过滤员工列表
 *
 * 读取搜索框中的文本，传入 loadData 实现模糊匹配过滤。
 */
void EmployeeDialog::onSearch()
{
    loadData(m_searchEdit->text());                          /*!< 按姓名关键字过滤并刷新表格 */
}
