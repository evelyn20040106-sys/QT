#include "visualizerwidget.h"   // 包含对应的头文件，获得类声明
#include <QPainter>             // 引入 QPainter，用于在控件上进行绘制
#include <QLinearGradient>      // 引入 QLinearGradient，用于创建线性渐变效果
#include <cmath>                // 引入标准数学库，使用 sin 等数学函数

/**
 * @brief 构造函数，初始化控件的默认属性并启动定时器连接。
 * @param parent 父窗口指针，传递给 QWidget 构造函数
 */
VisualizerWidget::VisualizerWidget(QWidget *parent)
    : QWidget(parent)            // 调用基类 QWidget 的构造函数，传入父指针
    , m_timer(new QTimer(this))  // 创建定时器对象，并将 this 设为其父对象（自动管理生命周期）
{
    setMinimumHeight(200);       // 设置控件的最小高度为 200 像素
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 设置尺寸策略为双向扩展，允许布局拉伸

    m_barHeights.resize(m_barCount);      // 将竖条当前高度数组大小调整为竖条总数
    m_targetHeights.resize(m_barCount);   // 将竖条目标高度数组大小调整为竖条总数
    for (int i = 0; i < m_barCount; ++i) { // 遍历所有竖条索引，从 0 到 m_barCount-1
        m_barHeights[i] = 0.0;             // 将当前高度初始化为 0（不可见状态）
        m_targetHeights[i] = 0.0;          // 将目标高度也初始化为 0
    }

    m_timer->setInterval(50); // 设置定时器触发间隔为 50 毫秒（约 20 帧 / 秒）
    connect(m_timer, &QTimer::timeout, this, [this]() { // 连接定时器的 timeout 信号到 Lambda 表达式
        updateHeights();   // 在 Lambda 中调用更新高度函数，计算下一帧的竖条数据
        update();          // 调用 QWidget::update() 触发控件的重绘事件
    });
}

/**
 * @brief 返回控件的推荐尺寸，供父布局或窗口参考。
 * @return 一个 QSize(400, 250)，表示推荐的宽度和高度
 */
QSize VisualizerWidget::sizeHint() const
{
    return QSize(400, 250);  // 返回推荐尺寸：宽 400 像素，高 250 像素
}

/**
 * @brief 设置可视化控件的激活状态，控制动画的启动与停止。
 * @param active true 表示激活（启动定时器，开始动画）；false 表示停用
 */
void VisualizerWidget::setActive(bool active)
{
    m_active = active;       // 将成员变量 m_active 更新为传入的 active 值
    if (active) {            // 如果设置为激活状态
        if (!m_timer->isActive()) // 检查定时器当前是否没有在运行
            m_timer->start();     // 如果未运行，则启动定时器，开始动画循环
    }
}

/**
 * @brief 重写 paintEvent，绘制音乐可视化频谱效果。
 * @param event 绘制事件对象（本实现中未使用）
 */
void VisualizerWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);        // 创建 QPainter 对象，绑定到当前控件上进行绘制
    painter.setRenderHint(QPainter::Antialiasing); // 启用抗锯齿渲染，使边缘更平滑
    painter.fillRect(rect(), QColor("#1a1a2e"));   // 用深蓝色填充整个控件区域作为背景

    int w = width();               // 获取控件的当前宽度（像素）
    int h = height();              // 获取控件的当前高度（像素）
    int barW = (w - 40) / m_barCount; // 计算每个竖条的宽度：总宽减去左右边距后除以竖条数量
    if (barW < 2) barW = 2;        // 如果计算出的竖条宽度小于 2 像素，则强制设为 2 像素（最小可见宽度）
    int gap = barW > 3 ? 2 : 1;    // 竖条之间的间隔：若条宽大于 3 像素则间隔 2 像素，否则间隔 1 像素
    int baseY = h - 30;            // 竖条底部基准线 Y 坐标（距底部 30 像素处）

    for (int i = 0; i < m_barCount; ++i) {  // 遍历所有竖条，逐个绘制
        qreal barH = m_barHeights[i] * (h - 60); // 根据当前高度值计算竖条的实际像素高度（可用高度为 h-60）

        if (barH < 2.0 && !m_active) // 如果竖条实际高度小于 2 像素且控件处于非激活状态
            continue;                // 跳过该竖条，不绘制（节省性能）

        int x = 20 + i * (barW + gap); // 计算当前竖条的左上角 X 坐标：左外边距 20 像素，加上之前竖条的宽度和间隔

        // Gradient color based on position and height
        // 根据竖条的位置和高度生成渐变色
        QColor color;               // 声明一个 QColor 对象用于存储当前竖条的颜色
        qreal ratio = (qreal)i / m_barCount; // 计算当前竖条在整个波段中的比例（0~1）
        if (ratio < 0.33)           // 如果位置比例小于 0.33（左 1/3 区域）
            color = QColor::fromHsv(140, 200, 180 + int(barH * 0.5));  // 生成青绿色系（色相 140）
        else if (ratio < 0.66)      // 如果位置比例在 0.33~0.66 之间（中间 1/3 区域）
            color = QColor::fromHsv(200, 200, 180 + int(barH * 0.5));  // 生成蓝色系（色相 200）
        else                        // 如果位置比例大于 0.66（右 1/3 区域）
            color = QColor::fromHsv(280, 180, 200 + int(barH * 0.5)); // 生成紫色系（色相 280）

        painter.setPen(Qt::NoPen);   // 设置画笔为无轮廓，竖条只填充不描边
        painter.setBrush(color);     // 设置画刷为前面计算得到的渐变色

        // Bar with rounded top
        // 绘制带有圆角的竖条
        QRectF barRect(x, baseY - barH, barW, barH);         // 构建竖条的矩形区域（左上角 x, y，宽 barW，高 barH）
        painter.drawRoundedRect(barRect, 2, 2);              // 绘制圆角矩形，圆角半径为 2 像素

        // Glow effect on top
        // 在竖条顶部绘制发光效果（一个小的半透明圆角矩形）
        if (barH > 5) {               // 只有竖条实际高度大于 5 像素时才绘制发光效果，避免太小时效果不明显
            QColor glow = color;      // 取当前竖条的颜色作为发光颜色的基础
            glow.setAlpha(80);        // 将发光颜色的透明度设为 80（半透明效果）
            painter.setBrush(glow);   // 设置画刷为半透明发光色
            QRectF glowRect(x - 1, baseY - barH - 2, barW + 2, 4); // 发光矩形：比竖条略宽，位于竖条顶部稍上方
            painter.drawRoundedRect(glowRect, 2, 2);                // 绘制圆角矩形作为发光效果
        }
    }

    // Gradient fade at bottom
    // 在控件底部绘制渐变淡出效果，使竖条底部自然融入背景
    QLinearGradient botGrad(0, h - 20, 0, h); // 创建从 (0, h-20) 到 (0, h) 的垂直线性渐变
    botGrad.setColorAt(0.0, QColor(26, 26, 46, 0));   // 渐变起始处：完全透明的深蓝色（alpha=0）
    botGrad.setColorAt(1.0, QColor("#1a1a2e"));       // 渐变结束处：不透明的深蓝色
    painter.fillRect(0, h - 20, w, 20, botGrad);      // 用该渐变填充控件底部 20 像素高的区域
}

/**
 * @brief 更新所有竖条的高度值，生成下一帧动画数据。
 *
 * 如果控件处于激活状态，则生成带有波浪模式和随机峰值的
 * 目标高度数组，并通过线性插值使当前高度平滑逼近目标；
 * 如果处于非激活状态，则将所有竖条的高度逐渐衰减至零。
 */
void VisualizerWidget::updateHeights()
{
    if (!m_active) {        // 如果控件当前处于非激活状态
        // Gradually settle to minimal
        // 逐渐将所有竖条的高度衰减至最小（零）
        for (int i = 0; i < m_barCount; ++i) { // 遍历所有竖条
            m_targetHeights[i] = 0.0;          // 将目标高度设为零
            m_barHeights[i] *= 0.85;           // 当前高度乘以 0.85 的衰减因子，逐渐趋近于零
        }
        return;              // 非激活状态下无需执行后续逻辑，直接返回
    }

    // Generate new targets with wave-like patterns
    // 生成新的目标高度，模拟波浪状的频谱效果
    for (int i = 0; i < m_barCount; ++i) { // 遍历所有竖条
        // Create a wave pattern that moves
        // 创建一个随时间移动的波浪模式
        qreal base = 0.1 + 0.15 * std::sin(i * 0.3 + m_barHeights[0] * 5); // 用正弦波生成基础高度，m_barHeights[0] 的变化使波移动
        qreal peak = QRandomGenerator::global()->bounded(0.6) + 0.1;         // 生成 0.1~0.7 之间的随机峰值

        // Peaks at different positions create a dynamic look
        // 在中间区域的竖条赋予更高的峰值，产生更动态的视觉效果
        if (i > m_barCount / 3 && i < m_barCount * 2 / 3) // 如果竖条索引在总条数的 1/3 到 2/3 之间（中间区域）
            peak *= 1.3;                                   // 将峰值放大 1.3 倍，使中间竖条更高

        m_targetHeights[i] = base + peak * 0.7; // 计算最终目标高度 = 基础波浪高度 + 随机峰值 * 0.7
    }

    // Smooth interpolation toward target
    // 通过平滑插值让当前高度向目标高度过渡
    for (int i = 0; i < m_barCount; ++i) {        // 遍历所有竖条
        qreal diff = m_targetHeights[i] - m_barHeights[i]; // 计算目标高度与当前高度的差值
        m_barHeights[i] += diff * 0.2;                      // 当前高度加上差值乘以 0.2 的步长，实现平滑逼近
        if (m_barHeights[i] < 0.01)                         // 如果当前高度小于 0.01 的极小阈值
            m_barHeights[i] = 0.0;                          // 将其直接归零，避免浮点误差导致残留
    }
}
