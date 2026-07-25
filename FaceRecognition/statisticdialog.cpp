/**
 * @file statisticdialog.cpp
 * @brief 打卡统计对话框实现文件，实现按日期/月份查询考勤记录的功能
 * @author 人脸识别考勤系统开发团队
 * @date 2026-07-08
 */

#include "statisticdialog.h" /*!< 统计对话框头文件 */
#include "databasemanager.h" /*!< 数据库管理器，提供考勤记录查询接口 */

#include <QFormLayout>  /*!< 表单布局 */
#include <QVBoxLayout>  /*!< 垂直布局 */
#include <QHBoxLayout>  /*!< 水平布局 */
#include <QHeaderView>  /*!< 表格头视图，用于设置列宽策略 */
#include <QGroupBox>    /*!< 分组框，用于将查询条件分组 */

/**
 * @brief 构造函数：初始化数据库引用，设置 UI 并默认加载当天记录
 * @param db     数据库管理器指针
 * @param parent 父窗口指针
 */
StatisticDialog::StatisticDialog(DatabaseManager *db, QWidget *parent)
    : QDialog(parent)  /*!< 调用基类构造函数 */
    , m_db(db)         /*!< 保存数据库管理器指针 */
{
    setupUI();                                                     /*!< 初始化界面控件布局 */
    /* 默认加载今天的打卡记录 */
    loadData(m_db->getRecordsByDate(QDate::currentDate().toString("yyyy-MM-dd")));
}

/**
 * @brief 初始化用户界面：创建查询条件分组、统计摘要标签和记录表格
 *
 * 界面布局从上到下依次为：
 * 1. 查询条件分组框（按日期查询 + 按月查询）
 * 2. 统计摘要标签（显示总记录数及各类状态的数量）
 * 3. 考勤记录表格（ID、姓名、部门、日期、上班时间、下班时间、状态）
 */
void StatisticDialog::setupUI()
{
    /* 设置窗口标题和最小尺寸 */
    setWindowTitle("打卡统计");
    setMinimumSize(800, 550);

    /* 创建主垂直布局 */
    auto *mainLayout = new QVBoxLayout(this);

    /* ---- 查询条件分组框 ---- */
    auto *queryGroup = new QGroupBox("查询条件");                  /*!< 分组框标题 */
    auto *queryLayout = new QVBoxLayout(queryGroup);               /*!< 分组框内的垂直布局 */

    /* ---- 按日期查询行 ---- */
    auto *dateLayout = new QHBoxLayout;
    dateLayout->addWidget(new QLabel("按日期查询:"));               /*!< 日期查询标签 */
    m_dateEdit = new QDateEdit(QDate::currentDate());              /*!< 日期选择控件，默认今天 */
    m_dateEdit->setCalendarPopup(true);                            /*!< 启用日历弹窗选择 */
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");                    /*!< 显示格式：年-月-日 */
    m_queryDateBtn = new QPushButton("查询");                      /*!< 日期查询按钮 */
    /* 按钮样式：蓝色背景、白色文字、圆角 */
    m_queryDateBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; "
                                   "padding: 6px 16px; border-radius: 4px; }");
    dateLayout->addWidget(m_dateEdit);                             /*!< 日期选择控件 */
    dateLayout->addWidget(m_queryDateBtn);                         /*!< 查询按钮 */
    dateLayout->addStretch();                                      /*!< 弹性空间，右对齐 */

    /* ---- 按月查询行 ---- */
    auto *monthLayout = new QHBoxLayout;
    monthLayout->addWidget(new QLabel("按月查询:"));               /*!< 月份查询标签 */
    m_monthEdit = new QDateEdit(QDate::currentDate());             /*!< 月份选择控件，默认当前月 */
    m_monthEdit->setCalendarPopup(true);                           /*!< 启用日历弹窗 */
    m_monthEdit->setDisplayFormat("yyyy-MM");                      /*!< 显示格式：年-月（只显示到月份） */
    m_queryMonthBtn = new QPushButton("查询");                     /*!< 月份查询按钮 */
    /* 按钮样式：蓝色背景、白色文字、圆角 */
    m_queryMonthBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; "
                                    "padding: 6px 16px; border-radius: 4px; }");
    monthLayout->addWidget(m_monthEdit);                           /*!< 月份选择控件 */
    monthLayout->addWidget(m_queryMonthBtn);                       /*!< 查询按钮 */
    monthLayout->addStretch();                                     /*!< 弹性空间，右对齐 */

    /* 将两行查询布局加入分组框的垂直布局 */
    queryLayout->addLayout(dateLayout);
    queryLayout->addLayout(monthLayout);

    /* ---- 统计摘要标签 ---- */
    m_summaryLabel = new QLabel("共 0 条记录");                    /*!< 初始显示无记录 */
    m_summaryLabel->setStyleSheet("font-weight: bold; color: #333; padding: 4px;"); /*!< 粗体深灰色文字 */

    /* ---- 考勤记录表格 ---- */
    m_table = new QTableWidget(this);                              /*!< 创建表格控件 */
    m_table->setColumnCount(7);                                    /*!< 设置 7 列 */
    m_table->setHorizontalHeaderLabels({                           /*!< 设置表头 */
        "ID", "姓名", "部门", "日期", "上班时间", "下班时间", "状态"
    });
    m_table->horizontalHeader()->setStretchLastSection(true);      /*!< 最后一列自动拉伸 */
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);  /*!< 点击选中整行 */
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   /*!< 禁止直接编辑 */
    m_table->setAlternatingRowColors(true);                        /*!< 交替行背景色 */

    /* 将各组件依次加入主布局 */
    mainLayout->addWidget(queryGroup);                             /*!< 查询条件分组 */
    mainLayout->addWidget(m_summaryLabel);                         /*!< 统计摘要 */
    mainLayout->addWidget(m_table);                                /*!< 记录表格 */

    /* ---- 连接信号槽 ---- */
    connect(m_queryDateBtn, &QPushButton::clicked, this, &StatisticDialog::onQueryByDate);
    connect(m_queryMonthBtn, &QPushButton::clicked, this, &StatisticDialog::onQueryByMonth);
}

/**
 * @brief 按日期查询槽函数：从日期控件获取日期并加载当天的打卡记录
 *
 * 将日期选择控件的值格式化为 "yyyy-MM-dd" 字符串，
 * 调用数据库按日期查询接口，将结果加载到表格中。
 */
void StatisticDialog::onQueryByDate()
{
    QString date = m_dateEdit->date().toString("yyyy-MM-dd");      /*!< 获取选中日期并格式化 */
    loadData(m_db->getRecordsByDate(date));                        /*!< 按日期查询并加载 */
}

/**
 * @brief 按月查询槽函数：从月份控件获取年月并加载该月的打卡记录
 *
 * 将月份选择控件的值格式化为 "yyyy-MM" 字符串，
 * 调用数据库按月查询接口，将结果加载到表格中。
 */
void StatisticDialog::onQueryByMonth()
{
    QString month = m_monthEdit->date().toString("yyyy-MM");       /*!< 获取选中月份并格式化 */
    loadData(m_db->getRecordsByMonth(month));                      /*!< 按月查询并加载 */
}

/**
 * @brief 将考勤记录加载到表格中，并更新统计摘要
 * @param records 考勤记录列表
 *
 * 遍历记录列表：
 * - 每行插入 ID、姓名、部门、日期、上班时间、下班时间
 * - 根据状态设置文本颜色：
 *   - "正常" → 绿色（Qt::darkGreen）
 *   - "迟到" → 暗黄色（Qt::darkYellow）
 *   - 含"早退" → 暗红色（Qt::darkRed）
 *   - 其他异常 → 红色（Qt::red）
 * - 统计各类记录的数量
 * - 更新摘要标签显示统计结果
 */
void StatisticDialog::loadData(const QList<AttendanceRecord> &records)
{
    m_table->setRowCount(0);                                        /*!< 清空表格所有行 */

    int normalCount = 0, lateCount = 0, earlyCount = 0, abnormalCount = 0; /*!< 各类状态计数器 */

    /* 遍历所有考勤记录，逐行插入表格 */
    for (int i = 0; i < records.size(); ++i) {
        const auto &r = records[i];                                /*!< 当前记录引用 */
        m_table->insertRow(i);                                     /*!< 插入新行 */
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(r.id)));           /*!< ID */
        m_table->setItem(i, 1, new QTableWidgetItem(r.employeeName));                   /*!< 姓名 */
        m_table->setItem(i, 2, new QTableWidgetItem(r.department));                     /*!< 部门 */
        m_table->setItem(i, 3, new QTableWidgetItem(r.date));                           /*!< 日期 */
        m_table->setItem(i, 4, new QTableWidgetItem(r.clockIn));                        /*!< 上班时间 */
        m_table->setItem(i, 5, new QTableWidgetItem(r.clockOut));                       /*!< 下班时间 */

        /* 状态列：根据状态值设置不同颜色并累加统计 */
        auto *statusItem = new QTableWidgetItem(r.status);
        if (r.status == "正常") {
            statusItem->setForeground(Qt::darkGreen);              /*!< 正常 → 绿色 */
            normalCount++;                                         /*!< 正常计数 +1 */
        } else if (r.status == "迟到") {
            statusItem->setForeground(Qt::darkYellow);             /*!< 迟到 → 暗黄色 */
            lateCount++;                                           /*!< 迟到计数 +1 */
        } else if (r.status.contains("早退")) {
            statusItem->setForeground(Qt::darkRed);                /*!< 早退 → 暗红色 */
            earlyCount++;                                          /*!< 早退计数 +1 */
        } else {
            statusItem->setForeground(Qt::red);                    /*!< 其他异常 → 红色 */
            abnormalCount++;                                       /*!< 异常计数 +1 */
        }
        m_table->setItem(i, 6, statusItem);                        /*!< 设置状态列 */
    }

    /* 更新统计摘要标签：总记录数及各类状态的数量分布 */
    m_summaryLabel->setText(
        QString("共 %1 条记录 | 正常: %2 | 迟到: %3 | 早退: %4 | 异常: %5")
            .arg(records.size())
            .arg(normalCount)
            .arg(lateCount)
            .arg(earlyCount)
            .arg(abnormalCount));
}
