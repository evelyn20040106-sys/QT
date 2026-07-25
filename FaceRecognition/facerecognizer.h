/**
 * @file facerecognizer.h
 * @brief 人脸识别器头文件，定义人脸识别相关的数据结构和接口
 * @author FaceRecognition Team
 * @date 2026-07-08
 */

#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QByteArray>

#include <opencv2/core.hpp>
#include <opencv2/face.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>

/**
 * @struct FaceResult
 * @brief 人脸识别结果结构体，存储检测和识别的完整结果信息
 */
struct FaceResult {
    bool detected = false;      ///< 是否检测到人脸
    int employeeId = -1;        ///< 识别出的员工 ID，-1 表示未识别
    QString name;               ///< 员工姓名
    double confidence = 0.0;    ///< 识别置信度（数值越小表示可信度越高）
    cv::Rect faceRect;          ///< 人脸在原始图像中的边界矩形
    QImage faceImage;           ///< 人脸区域的图像数据
};

/**
 * @class FaceRecognizer
 * @brief 人脸识别器类，提供人脸检测、特征提取、模型训练和识别功能
 * 
 * 基于 OpenCV 的 LBPH（局部二值模式直方图）算法实现人脸识别。
 * 使用 Haar 级联分类器进行人脸检测，LBPH 模型进行人脸识别。
 */
class FaceRecognizer : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent Qt 父对象指针，默认为 nullptr
     */
    explicit FaceRecognizer(QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~FaceRecognizer();

    /**
     * @brief 初始化人脸识别器，加载 Haar 级联分类器并创建 LBPH 识别模型
     * @param cascadePath Haar 级联分类器文件路径（如 haarcascade_frontalface_default.xml）
     * @return 初始化成功返回 true，失败返回 false
     */
    bool initialize(const QString &cascadePath);

    /**
     * @brief 检查识别器是否已成功初始化
     * @return 已初始化返回 true，否则返回 false
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief 在图像帧中检测所有人脸
     * @param frame 输入图像帧（彩色或灰度图）
     * @return 检测到的人脸边界矩形列表
     */
    QList<cv::Rect> detectFaces(const cv::Mat &frame);

    /**
     * @brief 识别图像帧中的人脸，返回识别结果
     * @param frame 输入图像帧
     * @return FaceResult 结构体，包含检测和识别结果
     */
    FaceResult recognize(const cv::Mat &frame);

    /**
     * @brief 从人脸图像中提取特征数据，用于后续训练
     * @param faceImage 人脸图像区域
     * @return 编码后的特征字节数组
     */
    QByteArray extractFeature(const cv::Mat &faceImage);

    /**
     * @brief 使用已注册的特征和标签训练 LBPH 识别模型
     * @param features 特征数据列表（由 extractFeature 生成）
     * @param labels   对应的员工 ID 标签列表
     * @return 训练成功返回 true，失败返回 false
     */
    bool trainRecognizer(const QList<QByteArray> &features, const QList<int> &labels);

    /**
     * @brief 将训练好的 LBPH 模型保存到文件
     * @param path 模型文件保存路径
     * @return 保存成功返回 true，失败返回 false
     */
    bool saveModel(const QString &path);

    /**
     * @brief 从文件加载已训练的 LBPH 模型
     * @param path 模型文件路径
     * @return 加载成功返回 true，失败返回 false
     */
    bool loadModel(const QString &path);

signals:
    /**
     * @brief 检测到人脸时发射的信号
     * @param rect 人脸在界面中的矩形区域
     */
    void faceDetected(const QRect &rect);

public:
    /**
     * @brief 对人脸图像进行预处理（灰度化、缩放、直方图均衡化）
     * @param face 原始人脸图像
     * @return 预处理后的标准尺寸人脸图像
     */
    cv::Mat preprocessFace(const cv::Mat &face);

private:
    /**
     * @brief 将 QImage 转换为 OpenCV 的 cv::Mat 格式
     * @param image 输入 QImage 图像
     * @return 转换后的 cv::Mat 图像
     */
    cv::Mat QImageToCvMat(const QImage &image);

    /**
     * @brief 将 OpenCV 的 cv::Mat 转换为 QImage 格式
     * @param mat 输入 cv::Mat 图像
     * @return 转换后的 QImage 图像
     */
    QImage CvMatToQImage(const cv::Mat &mat);

    cv::CascadeClassifier m_faceCascade;                   ///< Haar 级联分类器，用于人脸检测
    cv::Ptr<cv::face::LBPHFaceRecognizer> m_recognizer;    ///< LBPH 人脸识别模型指针
    bool m_initialized = false;                            ///< 初始化状态标志
};

#endif // FACERECOGNIZER_H
