/**
 * @file mainwindow.cpp
 * @brief 主窗口类的实现文件，包含人脸识别考勤系统的所有核心业务逻辑
 * @author 开发团队
 * @date 2026-07-08
 */

#include "mainwindow.h"
#include "facerecognizer.h"
#include "databasemanager.h"
#include "registerdialog.h"
#include "employeedialog.h"
#include "statisticdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QHeaderView>
#include <QPainter>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

/**
 * @brief 构造函数
 * 创建人脸识别器和数据库管理器实例，调用 setupUI() 构建界面，调用 initSystem() 初始化系统
 * @param parent 父级 QWidget 指针，传递给 QMainWindow
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建人脸识别器对象，父对象设置为 this，自动管理内存
    m_recognizer = new FaceRecognizer(this);
    // 创建数据库管理器对象
    m_db = new DatabaseManager(this);

    // 构建用户界面
    setupUI();
    // 初始化系统组件（数据库、模型、摄像头等）
    initSystem();
}

/**
 * @brief 析构函数
 * 如果摄像头已打开，则释放摄像头资源
 */
MainWindow::~MainWindow()
{
    // 检查摄像头是否处于打开状态
    if (m_cap.isOpened()) {
        // 释放摄像头
        m_cap.release();
    }
}

/**
 * @brief 设置主窗口的 UI 布局与控件
 *
 * 创建左右分栏布局：左侧显示摄像头画面和识别信息，右侧放置控制按钮和打卡记录列表。
 * 连接各按钮的点击信号到对应的槽函数。
 */
void MainWindow::setupUI()
{
    // 设置窗口标题和最小尺寸
    setWindowTitle("人脸识别考勤系统");
    setMinimumSize(1100, 650);

    // 创建中央部件和水平主布局
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto *mainLayout = new QHBoxLayout(centralWidget);

    // === 左侧面板：摄像头预览 ===
    auto *leftPanel = new QWidget;
    auto *leftLayout = new QVBoxLayout(leftPanel);

    // 摄像头画面显示标签，初始显示"加载中"提示
    m_cameraLabel = new QLabel("摄像头加载中...");
    m_cameraLabel->setAlignment(Qt::AlignCenter);
    m_cameraLabel->setMinimumSize(480, 360);
    // 设置深色边框样式
    m_cameraLabel->setStyleSheet(
        "border: 2px solid #333; background: #111; color: white; "
        "border-radius: 8px;");

    // 当前识别到的员工信息标签
    m_currentUserLabel = new QLabel("当前识别: 未识别");
    m_currentUserLabel->setAlignment(Qt::AlignCenter);
    m_currentUserLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; padding: 8px; "
        "background: #e3f2fd; border-radius: 4px;");

    // 操作提示信息标签
    m_infoLabel = new QLabel("将人脸对准摄像头进行识别");
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("color: #666; padding: 4px;");

    // 将左侧控件添加到垂直布局中
    leftLayout->addWidget(m_cameraLabel);
    leftLayout->addWidget(m_currentUserLabel);
    leftLayout->addWidget(m_infoLabel);

    // === 右侧面板：控制按钮 ===
    auto *rightPanel = new QWidget;
    auto *rightLayout = new QVBoxLayout(rightPanel);

    // 控制面板标题
    auto *titleLabel = new QLabel("考勤系统控制面板");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; "
                               "color: #1565C0; padding: 8px 0;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // 打卡按钮行（上班打卡 + 下班打卡）
    auto *clockLayout = new QHBoxLayout;
    m_clockInBtn = new QPushButton("⏰ 上班打卡");
    m_clockOutBtn = new QPushButton("⏰ 下班打卡");
    // 上班打卡按钮样式：绿色
    m_clockInBtn->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; "
        "padding: 12px 20px; font-size: 14px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #388E3C; }");
    // 下班打卡按钮样式：橙色
    m_clockOutBtn->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; "
        "padding: 12px 20px; font-size: 14px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #F57C00; }");
    clockLayout->addWidget(m_clockInBtn);
    clockLayout->addWidget(m_clockOutBtn);

    // 管理按钮行（人脸注册 + 员工管理 + 打卡统计）
    auto *manageLayout = new QHBoxLayout;
    m_registerBtn = new QPushButton("👤 人脸注册");
    m_manageBtn = new QPushButton("📋 员工管理");
    m_statsBtn = new QPushButton("📊 打卡统计");
    // 注册按钮样式：蓝色
    m_registerBtn->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; "
        "padding: 10px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #1976D2; }");
    // 管理按钮样式：紫色
    m_manageBtn->setStyleSheet(
        "QPushButton { background-color: #9C27B0; color: white; "
        "padding: 10px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #7B1FA2; }");
    // 统计按钮样式：蓝灰色
    m_statsBtn->setStyleSheet(
        "QPushButton { background-color: #607D8B; color: white; "
        "padding: 10px 16px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #455A64; }");
    manageLayout->addWidget(m_registerBtn);
    manageLayout->addWidget(m_manageBtn);
    manageLayout->addWidget(m_statsBtn);

    // 系统状态标签
    m_statusLabel = new QLabel("系统初始化中...");
    m_statusLabel->setStyleSheet("color: #666; font-style: italic; padding: 4px;");

    // 今日打卡记录列表标题
    auto *listLabel = new QLabel("今日打卡记录:");
    listLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding-top: 8px;");

    // 今日打卡记录树形控件，显示姓名、上班、下班、状态四列
    m_todayList = new QTreeWidget;
    m_todayList->setHeaderLabels({"姓名", "上班", "下班", "状态"});
    m_todayList->setAlternatingRowColors(true);  // 交替行颜色，提升可读性
    m_todayList->setRootIsDecorated(false);       // 不显示根节点展开图标

    // 将右侧控件添加到垂直布局
    rightLayout->addWidget(titleLabel);
    rightLayout->addLayout(clockLayout);
    rightLayout->addLayout(manageLayout);
    rightLayout->addWidget(m_statusLabel);
    rightLayout->addWidget(listLabel);
    rightLayout->addWidget(m_todayList, 1);  // 1 表示伸缩因子，使列表占据剩余空间

    // === 将左右面板通过 QSplitter 添加到主布局 ===
    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);  // 左侧面板占比 3
    splitter->setStretchFactor(1, 2);  // 右侧面板占比 2
    mainLayout->addWidget(splitter);

    // 连接各按钮的点击信号到对应的槽函数
    connect(m_clockInBtn, &QPushButton::clicked, this, &MainWindow::onClockIn);
    connect(m_clockOutBtn, &QPushButton::clicked, this, &MainWindow::onClockOut);
    connect(m_registerBtn, &QPushButton::clicked, this, &MainWindow::onRegister);
    connect(m_manageBtn, &QPushButton::clicked, this, &MainWindow::onManage);
    connect(m_statsBtn, &QPushButton::clicked, this, &MainWindow::onStatistics);
}

/**
 * @brief 初始化系统
 *
 * 依次完成：数据库初始化 → 人脸检测模型加载 → 摄像头打开 → 模型训练 → 定时器启动 → 记录加载。
 * 任何一步失败都会显示错误提示并中止后续初始化。
 */
void MainWindow::initSystem()
{
    // 更新状态标签为初始化中
    m_statusLabel->setText("系统初始化中...");

    // 第一步：初始化数据库
    if (!m_db->initialize()) {
        // 数据库初始化失败，显示严重错误对话框
        QMessageBox::critical(this, "错误", "数据库初始化失败!");
        return;
    }

    // 第二步：初始化人脸识别器，加载 Haar Cascade 级联分类器
    QString cascadePath = getHaarCascadePath();
    if (!m_recognizer->initialize(cascadePath)) {
        // 模型加载失败，显示警告对话框并告知路径
        QMessageBox::warning(this, "警告",
            "人脸检测模型加载失败!\n请确保 Haarcascade 文件存在。\n路径: " + cascadePath);
        return;
    }

    // 第三步：打开默认摄像头（设备索引 0）
    m_cap.open(0);
    if (!m_cap.isOpened()) {
        // 摄像头打开失败，更新状态为红色错误提示
        m_statusLabel->setText("❌ 无法打开摄像头");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        QMessageBox::warning(this, "警告", "无法打开摄像头，请检查设备连接");
        return;
    }

    // 第四步：使用数据库中已有的人脸数据重新训练识别模型
    retrainModel();

    // 第五步：启动摄像头画面更新定时器，约 30 FPS（1000ms / 33ms ≈ 30）
    m_cameraTimer = new QTimer(this);
    connect(m_cameraTimer, &QTimer::timeout, this, &MainWindow::onUpdateCamera);
    m_cameraTimer->start(33);

    // 第六步：启动人脸识别定时器，每 2 秒执行一次识别
    m_recognitionTimer = new QTimer(this);
    connect(m_recognitionTimer, &QTimer::timeout, this, &MainWindow::onRecognizeFace);
    m_recognitionTimer->start(2000);

    // 第七步：加载并显示今日打卡记录
    updateTodayList();

    // 标记系统已初始化完成
    m_initialized = true;
    // 更新状态为就绪
    m_statusLabel->setText("✅ 系统就绪 - 请对准摄像头");
    m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
}

/**
 * @brief 使用数据库中已有的人脸特征数据重新训练识别模型
 *
 * 从数据库获取所有员工的人脸特征向量和对应的标签（员工 ID），
 * 调用 FaceRecognizer::trainRecognizer() 进行训练。
 */
void MainWindow::retrainModel()
{
    // 获取所有员工信息
    auto employees = m_db->getAllEmployees();
    // 存储人脸特征数据
    QList<QByteArray> features;
    // 存储对应的标签（员工 ID）
    QList<int> labels;

    // 遍历员工列表，提取有人脸特征的数据
    for (const auto &emp : employees) {
        // 只添加包含人脸特征的员工
        if (!emp.faceFeature.isEmpty()) {
            features.append(emp.faceFeature);
            labels.append(emp.id);
        }
    }

    // 如果有有效的人脸数据，则执行训练
    if (!features.isEmpty()) {
        m_recognizer->trainRecognizer(features, labels);
        // 输出训练日志：训练了多少张人脸
        qDebug() << "Model trained with" << features.size() << "faces";
    }
}

/**
 * @brief 查找 Haar Cascade 级联分类器文件的路径
 *
 * 按照优先级依次检查多个可能的存放路径，返回第一个存在的文件路径。
 * 如果所有路径都不存在，则返回默认的文件名（由用户自行放置）。
 * @return 级联分类器文件的完整路径或默认文件名
 */
QString MainWindow::getHaarCascadePath()
{
    // 定义候选搜索路径列表，按优先级从高到低排列
    QStringList searchPaths = {
        // 1. 应用程序同级目录
        QCoreApplication::applicationDirPath() + "/haarcascade_frontalface_default.xml",
        // 2. OpenCV 4.10.0 构建目录
        "E:/opencv_build/opencv-4.10.0/data/haarcascades/haarcascade_frontalface_default.xml",
        // 3. OpenCV 级联文件目录
        "E:/opencv/etc/haarcascades/haarcascade_frontalface_default.xml",
        // 4. C 盘 OpenCV 级联文件目录
        "C:/opencv/etc/haarcascades/haarcascade_frontalface_default.xml",
    };

    // 遍历搜索路径，返回第一个存在的文件
    for (const auto &path : searchPaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    // 所有路径均未找到，返回默认文件名（用户需要自行下载放置）
    return "haarcascade_frontalface_default.xml";
}

/**
 * @brief 摄像头画面更新槽函数
 *
 * 从摄像头读取一帧图像，检测人脸并绘制绿色矩形框，
 * 如果已识别到员工，则在其脸部上方绘制姓名标签。
 * 最后将处理后的图像转换为 QImage 显示在界面上。
 */
void MainWindow::onUpdateCamera()
{
    // 如果摄像头未打开，直接返回
    if (!m_cap.isOpened()) return;

    // 从摄像头读取一帧图像
    cv::Mat frame;
    m_cap >> frame;
    // 如果读取到空帧，直接返回
    if (frame.empty()) return;

    // 检测当前帧中的人脸位置
    auto faces = m_recognizer->detectFaces(frame);
    // 克隆原帧，在副本上绘制（避免影响原始数据）
    cv::Mat display = frame.clone();

    // 遍历检测到的人脸矩形
    for (const auto &face : faces) {
        // 在脸部周围绘制绿色矩形框，线宽为 2
        cv::rectangle(display, face, cv::Scalar(0, 255, 0), 2);
    }

    // 将 OpenCV 的 BGR 图像转换为 RGB 格式以便 Qt 显示
    cv::Mat rgb;
    cv::cvtColor(display, rgb, cv::COLOR_BGR2RGB);
    // 转换为 QImage，使用 .copy() 确保数据所有权独立
    QImage qImg = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();

    // 如果已识别到员工，使用 QPainter 在图像上绘制中文姓名
    // （cv::putText 不支持中文，因此使用 Qt 的 QPainter）
    if (m_lastRecognizedId > 0) {
        // 从数据库获取员工信息
        auto emp = m_db->getEmployee(m_lastRecognizedId);
        if (emp.id > 0 && !faces.isEmpty()) {
            // 使用 QPainter 绘制中文文本
            QPainter painter(&qImg);
            // 设置字体：微软雅黑，14px 加粗
            QFont font("Microsoft YaHei", 14, QFont::Bold);
            painter.setFont(font);
            // 设置画笔颜色为绿色
            painter.setPen(QColor(0, 255, 0));

            // 在每个人脸矩形上方绘制员工姓名
            const cv::Rect &face = faces[0];
            // 将 OpenCV 坐标转换为 QImage 坐标
            int textX = face.x;
            int textY = (face.y - 8 > 0) ? face.y - 8 : face.y + face.height + 20;
            painter.drawText(textX, textY, emp.name);
            painter.end();
        }
    }

    // 缩放图像以适应标签尺寸，并保持宽高比，然后显示
    m_cameraLabel->setPixmap(QPixmap::fromImage(qImg.scaled(
        m_cameraLabel->width(), m_cameraLabel->height(), Qt::KeepAspectRatio)));
}

/**
 * @brief 人脸识别槽函数
 *
 * 定时从摄像头抓取一帧图像进行人脸识别。
 * 如果识别到已知员工，更新界面显示员工姓名、部门和置信度；
 * 如果检测到人脸但未识别（陌生人），显示提示信息。
 */
void MainWindow::onRecognizeFace()
{
    // 如果摄像头未打开，直接返回
    if (!m_cap.isOpened()) return;

    // 从摄像头读取一帧图像
    cv::Mat frame;
    m_cap >> frame;
    // 如果读取到空帧，直接返回
    if (frame.empty()) return;

    // 执行人脸识别，返回识别结果（是否检测到人脸、员工 ID、置信度）
    auto result = m_recognizer->recognize(frame);

    // 检查是否检测到人脸且识别到了已知员工
    if (result.detected && result.employeeId >= 0) {
        // 从数据库获取员工详细信息
        auto emp = m_db->getEmployee(result.employeeId);
        if (emp.id > 0) {
            // 保存识别到的员工 ID
            m_lastRecognizedId = emp.id;
            // 更新界面标签：显示员工姓名和部门
            m_currentUserLabel->setText(
                QString("✅ 当前识别: %1 (%2)")
                    .arg(emp.name)
                    .arg(emp.department));
            // 切换为绿色背景，表示识别成功
            m_currentUserLabel->setStyleSheet(
                "font-size: 16px; font-weight: bold; padding: 8px; "
                "background: #C8E6C9; border-radius: 4px;");
            // 显示识别置信度
            m_infoLabel->setText(
                QString("置信度: %1%")
                    .arg(100.0 - result.confidence, 0, 'f', 1));
        }
    } else if (result.detected) {
        // 检测到人脸但未匹配到已知员工（陌生人）
        m_currentUserLabel->setText("⚠️ 识别到陌生人 - 请先注册");
        m_currentUserLabel->setStyleSheet(
            "font-size: 16px; font-weight: bold; padding: 8px; "
            "background: #FFF9C4; border-radius: 4px;");
        // 重置识别的员工 ID 为 -1
        m_lastRecognizedId = -1;
    }
}

/**
 * @brief 上班打卡槽函数
 *
 * 验证当前是否有已识别的员工，检查是否已打过上班卡，
 * 若未打卡则调用数据库接口记录上班时间。
 */
void MainWindow::onClockIn()
{
    // 检查是否有已识别的员工（ID <= 0 表示未识别或陌生人）
    if (m_lastRecognizedId <= 0) {
        // 更新状态标签提示用户先进行人脸识别
        m_statusLabel->setText("❌ 请先进行人脸识别");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        return;
    }

    // 从数据库获取员工信息
    auto emp = m_db->getEmployee(m_lastRecognizedId);
    // 如果员工 ID 无效，直接返回
    if (emp.id <= 0) return;

    // 查询该员工今日的打卡记录
    auto todayRecord = m_db->getTodayRecord(emp.id);
    // 如果已有上班打卡记录，提示用户
    if (!todayRecord.clockIn.isEmpty()) {
        QMessageBox::information(this, "提示",
            QString("%1 今天已经打过卡了\n上班时间: %2")
                .arg(emp.name).arg(todayRecord.clockIn));
        return;
    }

    // 调用数据库接口执行上班打卡
    if (m_db->clockIn(emp.id)) {
        // 打卡成功，更新状态标签
        m_statusLabel->setText(QString("✅ %1 上班打卡成功!").arg(emp.name));
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
        // 刷新今日打卡记录列表
        updateTodayList();
    }
}

/**
 * @brief 下班打卡槽函数
 *
 * 验证当前是否有已识别的员工，检查是否有上班记录且未打下班卡，
 * 若满足条件则调用数据库接口记录下班时间。
 */
void MainWindow::onClockOut()
{
    // 检查是否有已识别的员工
    if (m_lastRecognizedId <= 0) {
        m_statusLabel->setText("❌ 请先进行人脸识别");
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        return;
    }

    // 从数据库获取员工信息
    auto emp = m_db->getEmployee(m_lastRecognizedId);
    // 如果员工 ID 无效，直接返回
    if (emp.id <= 0) return;

    // 查询该员工今日的打卡记录
    auto todayRecord = m_db->getTodayRecord(emp.id);
    // 如果没有上班打卡记录，提示先打上班卡
    if (todayRecord.clockIn.isEmpty()) {
        QMessageBox::warning(this, "提示",
            QString("%1 还没有上班打卡记录").arg(emp.name));
        return;
    }

    // 如果已经打过下班卡了，提示用户
    if (!todayRecord.clockOut.isEmpty()) {
        QMessageBox::information(this, "提示",
            QString("%1 今天已经打过下班卡了\n下班时间: %2")
                .arg(emp.name).arg(todayRecord.clockOut));
        return;
    }

    // 调用数据库接口执行下班打卡
    if (m_db->clockOut(emp.id)) {
        // 打卡成功
        m_statusLabel->setText(QString("✅ %1 下班打卡成功!").arg(emp.name));
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
        // 刷新今日打卡记录列表
        updateTodayList();
    }
}

/**
 * @brief 打开人脸注册对话框
 *
 * 弹出注册对话框，供新员工录入人脸信息。
 * 如果对话框以 Accept 状态关闭，则重新训练模型并刷新打卡记录列表。
 */
void MainWindow::onRegister()
{
    // 暂停主窗口的摄像头和识别定时器，避免与注册对话框抢摄像头
    if (m_cameraTimer) m_cameraTimer->stop();
    if (m_recognitionTimer) m_recognitionTimer->stop();

    // 释放主窗口的摄像头设备，让注册对话框能独占使用摄像头
    if (m_cap.isOpened()) m_cap.release();

    // 创建注册对话框，传入识别器和数据库管理器指针
    RegisterDialog dlg(m_recognizer, m_db, this);
    // 如果用户确认注册（对话框返回 Accepted）
    if (dlg.exec() == QDialog::Accepted) {
        // 重新训练模型以包含新注册的人脸
        retrainModel();
        // 刷新今日打卡记录列表
        updateTodayList();
    }

    // 先手动释放对话框的摄像头，确保主窗口能独占摄像头
    dlg.closeCamera();

    // 重新打开摄像头
    if (!m_cap.isOpened()) m_cap.open(0);

    // 恢复主窗口的摄像头和识别定时器
    if (m_cameraTimer) m_cameraTimer->start(33);
    if (m_recognitionTimer) m_recognitionTimer->start(2000);
}

/**
 * @brief 打开员工管理对话框
 *
 * 弹出员工管理对话框，供管理员查看/编辑/删除员工信息。
 * 关闭对话框后刷新打卡记录列表。
 */
void MainWindow::onManage()
{
    // 创建员工管理对话框，传入数据库管理器指针
    EmployeeDialog dlg(m_db, this);
    // 显示模态对话框
    dlg.exec();
    // 刷新今日打卡记录列表（可能有员工信息变更）
    updateTodayList();
}

/**
 * @brief 打开打卡统计对话框
 *
 * 弹出统计对话框，查看考勤统计数据和报表。
 */
void MainWindow::onStatistics()
{
    // 创建统计对话框，传入数据库管理器指针
    StatisticDialog dlg(m_db, this);
    // 显示模态对话框
    dlg.exec();
}

/**
 * @brief 刷新今日打卡记录列表
 *
 * 从数据库获取今日的所有打卡记录，逐条添加到 QTreeWidget 中，
 * 并根据考勤状态（正常/迟到/异常）设置不同的文字颜色。
 */
void MainWindow::updateTodayList()
{
    // 清空列表中的所有现有记录
    m_todayList->clear();
    // 获取今天的日期字符串，格式为 yyyy-MM-dd
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    // 从数据库查询今日打卡记录
    auto records = m_db->getRecordsByDate(today);

    // 遍历每条记录，添加到树形控件中
    for (const auto &r : records) {
        // 创建新的树形条目
        auto *item = new QTreeWidgetItem;
        // 设置四列数据：姓名、上班时间、下班时间、状态
        item->setText(0, r.employeeName);
        item->setText(1, r.clockIn);
        item->setText(2, r.clockOut);
        item->setText(3, r.status);

        // 根据状态设置不同的文字颜色
        if (r.status == "正常") {
            // 正常状态：深绿色
            item->setForeground(3, QColor(Qt::darkGreen));
        } else if (r.status == "迟到") {
            // 迟到状态：暗黄色
            item->setForeground(3, QColor(Qt::darkYellow));
        } else {
            // 异常/其他状态：红色
            item->setForeground(3, QColor(Qt::red));
        }

        // 将条目添加到树形控件中
        m_todayList->addTopLevelItem(item);
    }
}
