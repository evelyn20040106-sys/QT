/**
 * @file registerdialog.h
 * @brief 人脸注册对话框头文件，定义注册界面类
 * @author 人脸识别考勤系统开发团队
 * @date 2026-07-08
 */

#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>       /*!< Qt 对话框基类 */
#include <QLineEdit>     /*!< 单行文本输入框 */
#include <QLabel>        /*!< 文本/图片显示标签 */
#include <QPushButton>   /*!< 按钮控件 */
#include <QComboBox>     /*!< 下拉选择框 */
#include <QTimer>        /*!< 定时器，用于定时刷新摄像头画面 */
#include <QPixmap>       /*!< 图片像素映射，用于在标签上显示图像 */

#include <opencv2/core.hpp>   /*!< OpenCV 核心数据结构，如 cv::Mat */
#include <opencv2/videoio.hpp> /*!< OpenCV 视频输入输出，如 cv::VideoCapture */

class FaceRecognizer;   /*!< 前向声明：人脸识别器类 */
class DatabaseManager;  /*!< 前向声明：数据库管理器类 */

/**
 * @class RegisterDialog
 * @brief 员工人脸注册对话框，提供摄像头预览、拍照、保存人脸特征等功能
 *
 * 用户通过此对话框打开摄像头捕获人脸，录入员工姓名和部门信息，
 * 提取人脸特征后保存至数据库，完成人脸注册流程。
 */
class RegisterDialog : public QDialog
{
    Q_OBJECT            /*!< Qt 元对象宏，支持信号槽机制 */

public:
    /**
     * @brief 构造函数
     * @param recognizer 人脸识别器指针，用于检测和提取人脸特征
     * @param db         数据库管理器指针，用于保存员工信息
     * @param parent     父窗口指针，默认为 nullptr
     */
    explicit RegisterDialog(FaceRecognizer *recognizer,
                            DatabaseManager *db,
                            QWidget *parent = nullptr);

    /**
     * @brief 析构函数，关闭摄像头并释放资源
     */
    ~RegisterDialog();

private slots:
    /**
     * @brief 定时器槽函数，实时从摄像头读取帧并显示预览画面
     *
     * 每 30 毫秒触发一次，从摄像头采集一帧图像，检测人脸并绘制矩形框，
     * 将处理后的图像显示在界面上。
     */
    void onStartRegister();

    /**
     * @brief 拍照槽函数，捕获当前帧中最大的人脸区域
     *
     * 检测当前帧中的人脸，选取面积最大的人脸进行裁剪和预处理，
     * 然后冻结摄像头画面，启用保存按钮。
     */
    void onCapture();

    /**
     * @brief 保存槽函数，将员工信息和人脸特征保存至数据库
     *
     * 验证姓名输入和非空检查后，提取人脸特征，构造员工信息对象，
     * 调用数据库接口完成持久化存储。
     */
    void onSave();

    /**
     * @brief 取消槽函数，关闭对话框并返回拒绝结果
     */
    void onCancel();

public:
    /**
     * @brief 关闭摄像头，停止并清理定时器
     *
     * 提供给 MainWindow 调用，确保摄像头在对话框析构前释放。
     */
    void closeCamera();

private:
    /**
     * @brief 初始化用户界面，布局控件并连接信号槽
     */
    void setupUI();

    /**
     * @brief 打开默认摄像头（索引 0），启动定时器刷新画面
     */
    void openCamera();

    /* ---- 核心业务对象 ---- */
    FaceRecognizer *m_recognizer;   /*!< 人脸识别器，提供人脸检测、预处理和特征提取功能 */
    DatabaseManager *m_db;          /*!< 数据库管理器，负责员工信息的增删改查 */

    /* ---- UI 控件 ---- */
    QLabel *m_cameraLabel;          /*!< 摄像头画面显示标签，展示实时视频或已捕获的人脸图像 */
    QLineEdit *m_nameEdit;          /*!< 员工姓名输入框 */
    QLineEdit *m_deptEdit;          /*!< 员工部门输入框 */
    QPushButton *m_captureBtn;      /*!< "拍照"按钮，点击捕获当前人脸 */
    QPushButton *m_saveBtn;         /*!< "保存"按钮，点击保存员工信息（拍照后启用） */
    QPushButton *m_cancelBtn;       /*!< "取消"按钮，点击关闭对话框 */
    QLabel *m_statusLabel;          /*!< 状态提示标签，显示操作指引或错误信息 */

    /* ---- 摄像头相关 ---- */
    cv::VideoCapture m_cap;         /*!< OpenCV 视频捕获对象，连接摄像头设备 */
    QTimer *m_timer;                /*!< 定时器，以固定间隔触发视频帧读取 */
    cv::Mat m_currentFrame;         /*!< 当前从摄像头读取的原始帧图像 */
    cv::Mat m_capturedFace;         /*!< 已捕获并裁剪的人脸区域图像（预处理后） */
    bool m_captured = false;        /*!< 标记是否已完成拍照，true 表示已捕获人脸 */
};

#endif // REGISTERDIALOG_H
