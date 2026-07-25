/**
 * @file registerdialog.cpp
 * @brief 人脸注册对话框实现文件，实现摄像头预览、拍照、保存等注册流程
 * @author 人脸识别考勤系统开发团队
 * @date 2026-07-08
 */

#include "registerdialog.h"   /*!< 注册对话框头文件 */
#include "facerecognizer.h"   /*!< 人脸识别器，提供人脸检测和特征提取 */
#include "databasemanager.h"  /*!< 数据库管理器，提供员工信息存储 */

#include <QVBoxLayout>   /*!< 垂直布局 */
#include <QHBoxLayout>   /*!< 水平布局 */
#include <QFormLayout>   /*!< 表单布局（标签-字段对） */
#include <QGroupBox>     /*!< 分组框，用于将相关控件分组 */
#include <QMessageBox>   /*!< 消息框，用于显示提示/警告/错误信息 */
#include <QDebug>        /*!< 调试输出 */

#include <opencv2/imgproc.hpp>   /*!< OpenCV 图像处理，如颜色空间转换 */
#include <opencv2/imgcodecs.hpp> /*!< OpenCV 图像编码/解码 */

/**
 * @brief 构造函数：初始化识别器、数据库引用，设置 UI 并打开摄像头
 * @param recognizer 人脸识别器指针
 * @param db         数据库管理器指针
 * @param parent     父窗口指针
 */
RegisterDialog::RegisterDialog(FaceRecognizer *recognizer,
                                DatabaseManager *db,
                                QWidget *parent)
    : QDialog(parent)          /*!< 调用基类构造函数 */
    , m_recognizer(recognizer) /*!< 保存人脸识别器指针 */
    , m_db(db)                 /*!< 保存数据库管理器指针 */
{
    setupUI();   /*!< 初始化界面控件布局 */
    openCamera(); /*!< 打开摄像头并启动预览 */
}

/**
 * @brief 析构函数：关闭摄像头，释放定时器等资源
 */
RegisterDialog::~RegisterDialog()
{
    closeCamera(); /*!< 关闭摄像头并清理定时器 */
}

/**
 * @brief 初始化用户界面：创建摄像头预览区、员工信息表单、状态标签和操作按钮
 *
 * 界面从上到下依次为：
 * 1. 摄像头画面预览标签
 * 2. 员工信息分组框（姓名、部门输入框）
 * 3. 状态提示标签
 * 4. 按钮行（拍照、保存、取消）
 */
void RegisterDialog::setupUI()
{
    /* 设置窗口标题和最小尺寸 */
    setWindowTitle("人脸注册");
    setMinimumSize(650, 500);

    /* 创建主垂直布局 */
    auto *mainLayout = new QVBoxLayout(this);

    /* ---- 摄像头预览标签 ---- */
    m_cameraLabel = new QLabel("摄像头加载中...");       /*!< 初始提示文字 */
    m_cameraLabel->setAlignment(Qt::AlignCenter);        /*!< 居中对齐 */
    m_cameraLabel->setMinimumSize(400, 300);             /*!< 设置最小显示区域 */
    m_cameraLabel->setStyleSheet("border: 2px solid #ccc; background: #222; color: white;"); /*!< 深色背景 + 白色文字 + 灰色边框 */

    /* ---- 员工信息表单分组框 ---- */
    auto *formGroup = new QGroupBox("员工信息");           /*!< 分组框标题 */
    auto *formLayout = new QFormLayout(formGroup);        /*!< 表单布局 */

    m_nameEdit = new QLineEdit;                           /*!< 姓名字段 */
    m_nameEdit->setPlaceholderText("请输入姓名");          /*!< 占位提示文字 */
    m_deptEdit = new QLineEdit;                           /*!< 部门字段 */
    m_deptEdit->setPlaceholderText("请输入部门");          /*!< 占位提示文字 */

    formLayout->addRow("姓名:", m_nameEdit);              /*!< 添加"姓名"行 */
    formLayout->addRow("部门:", m_deptEdit);              /*!< 添加"部门"行 */

    /* ---- 状态提示标签 ---- */
    m_statusLabel = new QLabel("请将人脸对准摄像头，点击\"拍照\"按钮"); /*!< 初始操作提示 */
    m_statusLabel->setStyleSheet("color: #666; font-weight: bold;");  /*!< 灰色粗体文字 */

    /* ---- 按钮行 ---- */
    auto *btnLayout = new QHBoxLayout;                    /*!< 水平按钮布局 */
    m_captureBtn = new QPushButton("📷 拍照");            /*!< 拍照按钮 */
    m_saveBtn = new QPushButton("💾 保存");               /*!< 保存按钮（初始禁用） */
    m_cancelBtn = new QPushButton("取消");                 /*!< 取消按钮 */

    m_saveBtn->setEnabled(false);                         /*!< 初始禁用保存按钮，拍照后启用 */
    /* 拍照按钮样式：蓝色背景、白色文字、圆角 */
    m_captureBtn->setStyleSheet("QPushButton { background-color: #2196F3; color: white; "
                                 "padding: 8px 20px; border-radius: 4px; }");
    /* 保存按钮样式：绿色背景，禁用时灰色 */
    m_saveBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; "
                              "padding: 8px 20px; border-radius: 4px; }"
                              "QPushButton:disabled { background-color: #ccc; }");

    /* 将三个按钮加入水平布局 */
    btnLayout->addWidget(m_captureBtn);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_cancelBtn);

    /* 将各组件依次加入主布局 */
    mainLayout->addWidget(m_cameraLabel);  /*!< 摄像头画面 */
    mainLayout->addWidget(formGroup);      /*!< 员工信息表单 */
    mainLayout->addWidget(m_statusLabel);  /*!< 状态提示 */
    mainLayout->addLayout(btnLayout);      /*!< 按钮行 */

    /* ---- 连接信号槽 ---- */
    connect(m_captureBtn, &QPushButton::clicked, this, &RegisterDialog::onCapture);
    connect(m_saveBtn, &QPushButton::clicked, this, &RegisterDialog::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &RegisterDialog::onCancel);
}

/**
 * @brief 打开默认摄像头（索引 0），启动实时画面刷新定时器
 *
 * 如果摄像头打开失败，在状态标签显示错误信息。
 * 定时器每 33 毫秒触发一次 onStartRegister，实现约 30 FPS 的预览帧率。
 */
void RegisterDialog::openCamera()
{
    m_cap.open(0);                                         /*!< 打开默认摄像头（设备索引 0） */
    if (!m_cap.isOpened()) {                               /*!< 检查摄像头是否成功打开 */
        m_statusLabel->setText("❌ 无法打开摄像头!");       /*!< 显示错误信息 */
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
        return;                                             /*!< 打开失败，提前返回 */
    }

    m_timer = new QTimer(this);                            /*!< 创建定时器对象 */
    connect(m_timer, &QTimer::timeout, this, &RegisterDialog::onStartRegister); /*!< 绑定超时槽函数 */
    m_timer->start(33);                                    /*!< 每 33 毫秒触发一次（约 30 FPS） */
}

/**
 * @brief 关闭摄像头并清理定时器资源
 *
 * 停止定时器、释放定时器对象，然后释放摄像头设备。
 */
void RegisterDialog::closeCamera()
{
    if (m_timer) {                                         /*!< 定时器是否存在 */
        m_timer->stop();                                   /*!< 停止定时器 */
        delete m_timer;                                    /*!< 删除定时器对象 */
        m_timer = nullptr;                                 /*!< 指针置空，防止悬空 */
    }
    if (m_cap.isOpened()) {                                /*!< 摄像头是否仍处于打开状态 */
        m_cap.release();                                   /*!< 释放摄像头设备 */
    }
}

/**
 * @brief 定时器槽函数：从摄像头读取一帧图像，检测人脸并显示预览
 *
 * 如果已经拍照（m_captured 为 true），则不再更新画面。
 * 从摄像头读取一帧 → 检测人脸并绘制绿色矩形框 → 转换为 RGB 色彩空间
 * → 转为 QImage 并在标签上显示（保持宽高比缩放）。
 * 如果检测到人脸，更新状态提示。
 */
void RegisterDialog::onStartRegister()
{
    /* 如果已拍照，冻结画面不再更新 */
    if (m_captured) return;

    cv::Mat frame;                                         /*!< 存储从摄像头读取的帧 */
    m_cap >> frame;                                        /*!< 从摄像头读取一帧图像 */
    if (frame.empty()) return;                             /*!< 空帧则跳过 */

    m_currentFrame = frame.clone();                        /*!< 保存当前帧的深拷贝供后续使用 */

    /* 检测当前帧中的人脸 */
    auto faces = m_recognizer->detectFaces(frame);
    cv::Mat display = frame.clone();                       /*!< 拷贝用于绘制显示 */

    /* 在每张人脸上绘制绿色矩形框 */
    for (const auto &face : faces) {
        cv::rectangle(display, face, cv::Scalar(0, 255, 0), 2); /*!< 绿色边框，线宽 2 */
    }

    /* 将 OpenCV 的 BGR 图像转换为 RGB 格式，以便 Qt 正确显示颜色 */
    cv::Mat rgb;
    cv::cvtColor(display, rgb, cv::COLOR_BGR2RGB);
    /* 构造 QImage（使用 .copy() 获取数据所有权，防止悬空指针） */
    QImage qImg = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    /* 缩放图像以适应标签尺寸，保持宽高比 */
    m_cameraLabel->setPixmap(QPixmap::fromImage(qImg.scaled(
        m_cameraLabel->width(), m_cameraLabel->height(), Qt::KeepAspectRatio)));

    /* 如果检测到人脸，更新状态提示 */
    if (!faces.empty()) {
        m_statusLabel->setText("✅ 检测到人脸，点击\"拍照\"按钮");
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    }
}

/**
 * @brief 拍照槽函数：从当前帧中检测并裁剪最大人脸区域
 *
 * 流程：
 * 1. 检查当前帧是否有效
 * 2. 检测人脸，如果没有检测到则提示用户调整位置
 * 3. 选取面积最大的人脸进行裁剪
 * 4. 对人脸图像进行预处理（归一化等）
 * 5. 在标签上显示处理后的人脸图像
 * 6. 启用保存按钮，禁用拍照按钮
 */
void RegisterDialog::onCapture()
{
    /* 检查当前是否有可用帧 */
    if (m_currentFrame.empty()) {
        m_statusLabel->setText("❌ 没有画面，请检查摄像头");
        m_statusLabel->setStyleSheet("color: red;");
        return;
    }

    /* 在当前帧中检测人脸 */
    auto faces = m_recognizer->detectFaces(m_currentFrame);
    if (faces.isEmpty()) {                                 /*!< 未检测到人脸 */
        m_statusLabel->setText("❌ 未检测到人脸，请调整位置");
        m_statusLabel->setStyleSheet("color: red;");
        return;
    }

    /* 选取面积最大的人脸（最可能是主要人脸） */
    cv::Rect largest = faces[0];                           /*!< 初始化为第一张人脸 */
    for (const auto &f : faces) {
        if (f.area() > largest.area()) largest = f;       /*!< 如果面积更大则更新 */
    }

    /* 从当前帧中裁剪出最大人脸区域并深拷贝 */
    m_capturedFace = m_currentFrame(largest).clone();
    m_captured = true;                                     /*!< 标记已拍照，冻结摄像头画面 */

    /* 对裁剪出的人脸进行预处理（灰度化、缩放等），用于显示 */
    cv::Mat processed = m_recognizer->preprocessFace(m_capturedFace);
    cv::Mat rgb;
    cv::cvtColor(processed, rgb, cv::COLOR_GRAY2RGB);     /*!< 灰度图转 RGB（Qt 需要 RGB） */
    QImage qImg = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    /* 在标签上显示处理后的人脸 */
    m_cameraLabel->setPixmap(QPixmap::fromImage(qImg.scaled(
        m_cameraLabel->width(), m_cameraLabel->height(), Qt::KeepAspectRatio)));

    m_saveBtn->setEnabled(true);                            /*!< 启用保存按钮 */
    m_captureBtn->setEnabled(false);                        /*!< 禁用拍照按钮，防止重复拍照 */
    m_statusLabel->setText("✅ 拍照成功，输入员工信息后点击\"保存\"");
    m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
}

/**
 * @brief 保存槽函数：验证输入，提取人脸特征，保存员工信息到数据库
 *
 * 流程：
 * 1. 校验员工姓名不能为空
 * 2. 校验已有人脸图像数据
 * 3. 提取人脸特征（特征向量）
 * 4. 构造员工信息对象（姓名、部门、人脸特征）
 * 5. 调用数据库接口保存
 * 6. 成功则弹出提示并关闭对话框，失败则显示错误信息
 */
void RegisterDialog::onSave()
{
    /* 校验：员工姓名不能为空 */
    QString name = m_nameEdit->text().trimmed();           /*!< 去除首尾空格 */
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入员工姓名");
        m_nameEdit->setFocus();                            /*!< 将焦点移回姓名输入框 */
        return;
    }

    /* 校验：是否已拍照 */
    if (m_capturedFace.empty()) {
        QMessageBox::warning(this, "提示", "请先拍照");
        return;
    }

    /* 从已捕获的人脸图像中提取特征数据 */
    QByteArray feature = m_recognizer->extractFeature(m_capturedFace);

    /* 构造员工信息并保存至数据库 */
    EmployeeInfo info;
    info.name = name;                                      /*!< 员工姓名 */
    info.department = m_deptEdit->text().trimmed();        /*!< 员工部门 */
    info.faceFeature = feature;                            /*!< 人脸特征数据 */

    /* 调用数据库接口添加员工记录 */
    if (m_db->addEmployee(info)) {
        QMessageBox::information(this, "成功", QString("员工 %1 注册成功!").arg(name));
        accept();                                           /*!< 关闭对话框并返回 Accepted */
    } else {
        QMessageBox::critical(this, "错误", "保存失败，请重试");
    }
}

/**
 * @brief 取消槽函数：关闭对话框并返回拒绝结果
 */
void RegisterDialog::onCancel()
{
    reject();                                              /*!< 关闭对话框并返回 Rejected */
}
