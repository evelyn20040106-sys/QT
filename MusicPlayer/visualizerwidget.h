#ifndef VISUALIZERWIDGET_H  // 防止头文件被重复包含的宏开关
#define VISUALIZERWIDGET_H  // 定义该宏，确保本文件只被编译一次

#include <QWidget>          // 引入 QWidget 基类，所有可视控件的基础
#include <QTimer>           // 引入 QTimer，用于驱动动画帧刷新
#include <QVector>          // 引入 QVector，用于存储动态数组数据
#include <QRandomGenerator> // 引入 QRandomGenerator，用于生成随机数

/**
 * @brief 一个简单的动画音乐可视化控件，用移动的彩色竖条模拟频谱效果。
 *
 * 该控件通过定时器定期更新每个竖条的目标高度，并通过插值平滑过渡，
 * 模拟出类似音乐频谱的动态视觉效果。控件支持激活 / 停用状态，
 * 停用后竖条会逐渐归零。
 */
class VisualizerWidget : public QWidget  // 继承自 QWidget，自定义绘制控件
{
    Q_OBJECT  // Qt 元对象宏，启用信号槽等元对象特性

public:
    /**
     * @brief 构造函数，初始化可视化控件。
     * @param parent 父窗口指针，默认为 nullptr，表示无父窗口
     */
    explicit VisualizerWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置可视化控件的激活状态。
     * @param active true 表示激活（启动动画），false 表示停用
     */
    void setActive(bool active);

    /**
     * @brief 返回控件的推荐尺寸，供布局管理器参考。
     * @return 推荐的控件尺寸 QSize(400, 250)
     */
    QSize sizeHint() const override;

protected:
    /**
     * @brief 重写绘制事件，完成所有自定义绘图。
     * @param event 绘制事件对象（本实现中未使用）
     */
    void paintEvent(QPaintEvent *event) override;

private:
    QTimer *m_timer;                    // 定时器，驱动动画帧的更新和重绘
    QVector<qreal> m_barHeights;        // 当前每个竖条的高度（归一化值 0~1）
    QVector<qreal> m_targetHeights;     // 每个竖条的目标高度（用于平滑插值）
    int m_barCount = 48;                // 竖条的总数量
    bool m_active = false;             // 控件是否处于激活（动画）状态

    /**
     * @brief 更新所有竖条的高度值，生成下一帧的数据。
     *
     * 在激活状态下生成波浪状随机目标高度，并通过线性插值平滑过渡；
     * 在非激活状态下将所有竖条高度逐渐衰减至零。
     */
    void updateHeights();
};

#endif // VISUALIZERWIDGET_H  // 结束头文件的条件编译块
