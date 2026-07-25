/**
 * @file mainwindow.h
 * @brief 主窗口类的头文件，定义了人脸识别考勤系统的主界面
 * @author Evelyn_冯
 * @date 2026-07-08
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QStackedWidget>
#include <QTreeWidget>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

/// 前向声明人脸识别器类
class FaceRecognizer;
/// 前向声明数据库管理器类
class DatabaseManager;

/**
 * @class MainWindow
 * @brief 系统主窗口类，负责整个考勤系统的界面布局与核心交互逻辑
 *
 * 包含摄像头预览、人脸识别、打卡操作、员工管理、统计查看等完整功能。
 * 继承自 QMainWindow，作为应用程序的主界面。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化识别器、数据库管理器，并设置界面
     * @param parent 父级 QWidget 指针，默认为 nullptr
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /// 析构函数，释放摄像头资源
    ~MainWindow();

private slots:
    /**
     * @brief 上班打卡槽函数
     * 检查当前识别到的员工是否已打过上班卡，若未打则记录上班时间
     */
    void onClockIn();

    /**
     * @brief 下班打卡槽函数
     * 检查当前识别到的员工是否有上班记录且未打下班卡，若满足则记录下班时间
     */
    void onClockOut();

    /**
     * @brief 打开人脸注册对话框槽函数
     * 弹出注册对话框，供新员工录入人脸信息
     */
    void onRegister();

    /**
     * @brief 打开员工管理对话框槽函数
     * 弹出员工管理对话框，查看/编辑员工信息
     */
    void onManage();

    /**
     * @brief 打开打卡统计对话框槽函数
     * 弹出统计对话框，查看考勤统计数据
     */
    void onStatistics();

    /**
     * @brief 摄像头画面更新槽函数
     * 定时从摄像头读取帧，检测人脸并绘制矩形框，更新到界面标签上
     */
    void onUpdateCamera();

    /**
     * @brief 人脸识别槽函数
     * 定时从摄像头抓取帧进行人脸识别，更新当前识别结果到界面
     */
    void onRecognizeFace();

private:
    /// 设置主窗口的 UI 布局与控件
    void setupUI();

    /// 初始化系统：数据库、人脸识别器、摄像头、定时器等
    void initSystem();

    /// 使用数据库中已有的人脸特征重新训练识别模型
    void retrainModel();

    /// 刷新今日打卡记录列表显示
    void updateTodayList();

    /**
     * @brief 查找 Haar Cascade 级联分类器文件的路径
     * @return 返回找到的第一个有效路径，若均未找到则返回默认文件名
     */
    QString getHaarCascadePath();

    /// 人脸识别器对象，负责检测与识别人脸
    FaceRecognizer *m_recognizer;

    /// 数据库管理器对象，负责员工与考勤数据的存取
    DatabaseManager *m_db;

    // ---------- 摄像头相关 ----------
    /// OpenCV 视频捕获对象，用于读取摄像头帧
    cv::VideoCapture m_cap;

    /// 摄像头画面更新定时器
    QTimer *m_cameraTimer;

    /// 人脸识别触发定时器
    QTimer *m_recognitionTimer;

    // ---------- UI 控件 ----------
    /// 摄像头画面显示标签
    QLabel *m_cameraLabel;

    /// 提示信息标签（如操作指引）
    QLabel *m_infoLabel;

    /// 系统状态显示标签
    QLabel *m_statusLabel;

    /// 当前识别到的员工信息标签
    QLabel *m_currentUserLabel;

    /// 上班打卡按钮
    QPushButton *m_clockInBtn;

    /// 下班打卡按钮
    QPushButton *m_clockOutBtn;

    /// 人脸注册按钮
    QPushButton *m_registerBtn;

    /// 员工管理按钮
    QPushButton *m_manageBtn;

    /// 打卡统计按钮
    QPushButton *m_statsBtn;

    /// 今日打卡记录树形列表控件
    QTreeWidget *m_todayList;

    // ---------- 状态变量 ----------
    /// 上次识别到的员工 ID（-1 表示未识别或陌生人）
    int m_lastRecognizedId = -1;

    /// 系统是否已成功初始化
    bool m_initialized = false;
};

#endif // MAINWINDOW_H
