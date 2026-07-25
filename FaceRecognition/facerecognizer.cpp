/**
 * @file facerecognizer.cpp
 * @brief 人脸识别器实现文件，实现人脸检测、特征提取、模型训练与识别等功能
 * @author FaceRecognition Team
 * @date 2026-07-08
 */

#include "facerecognizer.h"
#include <QDebug>
#include <QFile>
#include <QDataStream>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

/**
 * @brief 构造函数，初始化父类 QObject
 * @param parent Qt 父对象指针，默认为 nullptr
 */
FaceRecognizer::FaceRecognizer(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 析构函数（默认实现）
 */
FaceRecognizer::~FaceRecognizer()
{
}

/**
 * @brief 初始化人脸识别器
 * 
 * 加载 Haar 级联分类器文件，并创建 LBPH 人脸识别模型。
 * 
 * @param cascadePath Haar 级联分类器 XML 文件路径
 * @return 初始化成功返回 true，失败（如级联文件加载失败）返回 false
 */
bool FaceRecognizer::initialize(const QString &cascadePath)
{
    // 加载 Haar 级联分类器用于人脸检测
    if (!m_faceCascade.load(cascadePath.toStdString())) {
        qWarning() << "Failed to load face cascade:" << cascadePath;
        return false;
    }

    // 创建 LBPH 人脸识别器
    // 参数：半径=1，邻域=8，网格X=8，网格Y=8，阈值=100.0
    m_recognizer = cv::face::LBPHFaceRecognizer::create(1, 8, 8, 8, 100.0);
    m_initialized = true;   // 标记为已初始化
    return true;
}

/**
 * @brief 在图像帧中检测所有人脸
 * 
 * 将输入图像转为灰度图，进行直方图均衡化后，使用 Haar 级联分类器检测人脸。
 * 
 * @param frame 输入图像帧（彩色 BGR 或灰度图）
 * @return 检测到的人脸边界矩形列表，未检测到时返回空列表
 */
QList<cv::Rect> FaceRecognizer::detectFaces(const cv::Mat &frame)
{
    QList<cv::Rect> faces;  // 存储检测结果
    cv::Mat gray;           // 灰度图

    // 将彩色图像转换为灰度图
    if (frame.channels() > 1) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = frame.clone();
    }

    // 直方图均衡化，提升光照变化下的人脸检测效果
    cv::equalizeHist(gray, gray);

    // 执行多尺度人脸检测
    std::vector<cv::Rect> detected;
    // scaleFactor=1.1, minNeighbors=3, flags=0, minSize=80x80
    m_faceCascade.detectMultiScale(gray, detected, 1.1, 3, 0, cv::Size(80, 80));

    // 将检测结果从 std::vector 转换为 QList
    for (const auto &rect : detected) {
        faces.append(rect);
    }

    return faces;
}

/**
 * @brief 识别图像帧中的人脸
 * 
 * 先检测人脸，然后对最大的人脸区域进行预处理和识别。
 * 如果已训练模型，则预测员工 ID 和置信度。
 * 
 * @param frame 输入图像帧
 * @return FaceResult 包含检测/识别结果的结构体
 */
FaceResult FaceRecognizer::recognize(const cv::Mat &frame)
{
    FaceResult result;              // 创建结果对象
    QList<cv::Rect> faces = detectFaces(frame);  // 检测人脸

    // 未检测到人脸，直接返回
    if (faces.isEmpty()) {
        result.detected = false;
        return result;
    }

    // 选取面积最大的脸作为主要识别目标
    cv::Rect largestFace = faces[0];
    for (const auto &face : faces) {
        if (face.area() > largestFace.area()) {
            largestFace = face;
        }
    }

    cv::Mat faceROI = frame(largestFace);     // 裁剪人脸区域
    cv::Mat processed = preprocessFace(faceROI);  // 预处理人脸图像

    result.detected = true;         // 标记已检测到人脸
    result.faceRect = largestFace;  // 记录人脸位置
    result.faceImage = CvMatToQImage(processed); // 转换为人脸图像

    // 如果模型已训练（有标签数据），则进行识别预测
    if (!m_recognizer->getLabels().empty()) {
        int label = -1;             // 预测标签（员工 ID）
        double confidence = 0.0;    // 置信度
        m_recognizer->predict(processed, label, confidence);

        // 有效标签（>=0）时记录识别结果
        if (label >= 0) {
            result.employeeId = label;
            result.confidence = confidence;
        }
    }

    return result;
}

/**
 * @brief 从人脸图像中提取特征数据
 * 
 * 将人脸图像预处理后编码为 BMP 字节流，用于后续模型训练。
 * LBPH 识别器在训练时内部自行完成特征提取。
 * 
 * @param faceImage 原始人脸图像区域
 * @return 编码后的图像字节数组
 */
QByteArray FaceRecognizer::extractFeature(const cv::Mat &faceImage)
{
    // 预处理人脸图像（灰度化、缩放、直方图均衡化）
    cv::Mat processed = preprocessFace(faceImage);

    // 对于 LBPH 算法，将预处理后的图像数据作为"特征"存储
    // LBPH 识别器在训练过程中内部处理特征提取
    std::vector<uchar> buf;              // 编码缓冲区
    cv::imencode(".bmp", processed, buf); // 将图像编码为 BMP 格式

    // 将 std::vector<uchar> 转换为 QByteArray 返回
    QByteArray data(reinterpret_cast<const char*>(buf.data()),
                    static_cast<int>(buf.size()));
    return data;
}

/**
 * @brief 使用已注册的特征和标签训练 LBPH 识别模型
 * 
 * 将 QByteArray 格式的特征数据解码为 cv::Mat 图像，
 * 构建训练数据集并训练 LBPH 模型。
 * 
 * @param features 特征数据列表（由 extractFeature 生成的编码图像数据）
 * @param labels   对应的员工 ID 标签列表
 * @return 训练成功返回 true，失败（数据为空或解码失败）返回 false
 */
bool FaceRecognizer::trainRecognizer(const QList<QByteArray> &features,
                                      const QList<int> &labels)
{
    // 校验特征与标签数量一致且非空
    if (features.size() != labels.size() || features.isEmpty()) {
        qWarning() << "FaceRecognizer: mismatched or empty features/labels";
        return false;
    }

    std::vector<cv::Mat> images;  // 训练图像集
    std::vector<int> labelVec;    // 标签集

    // 遍历所有特征数据，解码为 OpenCV 图像
    for (int i = 0; i < features.size(); ++i) {
        // 将 QByteArray 转换为 std::vector<uchar>
        std::vector<uchar> buf(features[i].begin(), features[i].end());
        // 从缓冲区解码为灰度图
        cv::Mat img = cv::imdecode(buf, cv::IMREAD_GRAYSCALE);
        if (!img.empty()) {
            images.push_back(img);      // 加入训练集
            labelVec.push_back(labels[i]); // 加入对应标签
        }
    }

    // 没有有效图像，训练失败
    if (images.empty()) {
        qWarning() << "FaceRecognizer: no valid images to train";
        return false;
    }

    // 使用图像和标签训练 LBPH 模型
    m_recognizer->train(images, labelVec);
    return true;
}

/**
 * @brief 将训练好的 LBPH 模型保存到文件
 * 
 * @param path 模型文件保存路径（通常为 .xml 或 .yaml 格式）
 * @return 保存成功返回 true，失败（未初始化或文件写入异常）返回 false
 */
bool FaceRecognizer::saveModel(const QString &path)
{
    // 检查识别器是否已创建
    if (!m_recognizer) return false;
    try {
        // 将模型数据写入文件
        m_recognizer->write(path.toStdString());
        return true;
    } catch (const cv::Exception &e) {
        // 捕获 OpenCV 异常并记录错误信息
        qWarning() << "Failed to save model:" << e.what();
        return false;
    }
}

/**
 * @brief 从文件加载已训练的 LBPH 模型
 * 
 * @param path 模型文件路径
 * @return 加载成功返回 true，失败（文件不存在或读取异常）返回 false
 */
bool FaceRecognizer::loadModel(const QString &path)
{
    // 检查模型文件是否存在
    if (!QFile::exists(path)) return false;
    try {
        // 从文件读取模型数据
        m_recognizer->read(path.toStdString());
        return true;
    } catch (const cv::Exception &e) {
        // 捕获 OpenCV 异常并记录错误信息
        qWarning() << "Failed to load model:" << e.what();
        return false;
    }
}

/**
 * @brief 对人脸图像进行预处理
 * 
 * 预处理流程：灰度化 → 缩放到 100x100 标准尺寸 → 直方图均衡化。
 * 标准化处理有助于提高识别准确率。
 * 
 * @param face 原始人脸图像（彩色或灰度图）
 * @return 预处理后的灰度人脸图像（100x100）
 */
cv::Mat FaceRecognizer::preprocessFace(const cv::Mat &face)
{
    cv::Mat gray;   // 存储灰度图

    // 彩色图转灰度图
    if (face.channels() > 1) {
        cv::cvtColor(face, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = face.clone();
    }

    // 缩放到 100x100 标准尺寸，确保输入模型的数据尺寸一致
    cv::Mat resized;
    cv::resize(gray, resized, cv::Size(100, 100));

    // 直方图均衡化，增强对比度，减少光照影响
    cv::equalizeHist(resized, resized);

    return resized;
}

/**
 * @brief 将 QImage 转换为 OpenCV 的 cv::Mat 格式
 * 
 * 支持 Format_RGB32、Format_ARGB32、Format_RGB888 和 Format_Grayscale8 格式。
 * 其他格式先转换为 Format_RGB888 再处理。
 * 
 * @param image 输入的 QImage 图像
 * @return 转换后的 cv::Mat 图像（BGR 或灰度格式）
 */
cv::Mat FaceRecognizer::QImageToCvMat(const QImage &image)
{
    cv::Mat mat;
    switch (image.format()) {
    // 32 位 RGB/ARGB 格式，转为 4 通道 CV_8UC4
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4,
                      const_cast<uchar*>(image.bits()), image.bytesPerLine());
        break;

    // 24 位 RGB 格式，转为 3 通道 CV_8UC3，然后从 RGB 转为 BGR
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3,
                      const_cast<uchar*>(image.bits()), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        break;

    // 8 位灰度图格式，转为单通道 CV_8UC1
    case QImage::Format_Grayscale8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1,
                      const_cast<uchar*>(image.bits()), image.bytesPerLine());
        break;

    // 其他未知格式：先转换为 Format_RGB888，再转为 BGR
    default:
        QImage converted = image.convertToFormat(QImage::Format_RGB888);
        mat = cv::Mat(converted.height(), converted.width(), CV_8UC3,
                      const_cast<uchar*>(converted.bits()), converted.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        break;
    }
    // 返回深拷贝，确保数据独立于原始 QImage
    return mat.clone();
}

/**
 * @brief 将 OpenCV 的 cv::Mat 转换为 QImage 格式
 * 
 * 支持 CV_8UC1（灰度）、CV_8UC3（BGR 转 RGB）和 CV_8UC4（ARGB）格式。
 * 
 * @param mat 输入的 cv::Mat 图像
 * @return 转换后的 QImage 图像；若格式不支持则返回空 QImage
 */
QImage FaceRecognizer::CvMatToQImage(const cv::Mat &mat)
{
    // 单通道灰度图 → Format_Grayscale8
    if (mat.type() == CV_8UC1) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step,
                      QImage::Format_Grayscale8).copy();
    // 三通道 BGR 图 → 转为 RGB → Format_RGB888
    } else if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step,
                      QImage::Format_RGB888).copy();
    // 四通道 ARGB 图 → Format_ARGB32
    } else if (mat.type() == CV_8UC4) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step,
                      QImage::Format_ARGB32).copy();
    }
    // 不支持的格式返回空 QImage
    return {};
}
