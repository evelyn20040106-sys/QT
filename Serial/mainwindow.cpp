#include "mainwindow.h"

#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() = default;

// 串口配置区域 - 端口、波特率、数据位、停止位、校验位 + 打开按钮
void MainWindow::setupUI()
{
    setWindowTitle("串口调试助手");
    resize(800, 600);

    // ---------- 中央部件 ----------
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ========== 1. 串口参数配置区 ==========
    auto *configGroup = new QGroupBox("串口配置");
    auto *configLayout = new QGridLayout(configGroup);

    // 端口号
    configLayout->addWidget(new QLabel("端口号:"), 0, 0);
    m_portCombo = new QComboBox;
    m_portCombo->setMinimumWidth(120);
    configLayout->addWidget(m_portCombo, 0, 1);

    // 波特率
    configLayout->addWidget(new QLabel("波特率:"), 0, 2);
    m_baudCombo = new QComboBox;
    m_baudCombo->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_baudCombo->setCurrentText("115200");
    configLayout->addWidget(m_baudCombo, 0, 3);

    // 数据位
    configLayout->addWidget(new QLabel("数据位:"), 0, 4);
    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentText("8");
    configLayout->addWidget(m_dataBitsCombo, 0, 5);

    // 停止位
    configLayout->addWidget(new QLabel("停止位:"), 0, 6);
    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    m_stopBitsCombo->setCurrentText("1");
    configLayout->addWidget(m_stopBitsCombo, 0, 7);

    // 校验位
    configLayout->addWidget(new QLabel("校验位:"), 0, 8);
    m_parityCombo = new QComboBox;
    m_parityCombo->addItems({"None", "Even", "Odd", "Mark", "Space"});
    configLayout->addWidget(m_parityCombo, 0, 9);

    // 打开/关闭按钮
    m_openBtn = new QPushButton("打开串口");
    m_openBtn->setMinimumWidth(90);
    configLayout->addWidget(m_openBtn, 0, 10);

    mainLayout->addWidget(configGroup);

    // ========== 2. 接收区 ==========
    auto *receiveGroup = new QGroupBox("接收区");
    auto *receiveLayout = new QVBoxLayout(receiveGroup);

    m_receiveEdit = new QTextEdit;
    m_receiveEdit->setReadOnly(true);
    m_receiveEdit->setPlaceholderText("接收到的数据将显示在这里...");
    receiveLayout->addWidget(m_receiveEdit);

    auto *receiveCtrlLayout = new QHBoxLayout;
    m_hexReceiveCheck = new QCheckBox("Hex 显示");
    receiveCtrlLayout->addWidget(m_hexReceiveCheck);
    receiveCtrlLayout->addStretch();
    m_clearReceiveBtn = new QPushButton("清空接收");
    receiveCtrlLayout->addWidget(m_clearReceiveBtn);
    receiveLayout->addLayout(receiveCtrlLayout);

    mainLayout->addWidget(receiveGroup, 1);    // 拉伸因子1：接收区占更多空间

    // ========== 3. 发送区 ==========
    auto *sendGroup = new QGroupBox("发送区");
    auto *sendLayout = new QVBoxLayout(sendGroup);

    m_sendEdit = new QTextEdit;
    m_sendEdit->setPlaceholderText("输入要发送的数据...");
    m_sendEdit->setMaximumHeight(120);
    sendLayout->addWidget(m_sendEdit);

    auto *sendCtrlLayout = new QHBoxLayout;
    m_hexSendCheck = new QCheckBox("Hex 发送");
    sendCtrlLayout->addWidget(m_hexSendCheck);
    sendCtrlLayout->addStretch();
    m_clearSendBtn = new QPushButton("清空发送");
    sendCtrlLayout->addWidget(m_clearSendBtn);
    m_sendBtn = new QPushButton("发送");
    m_sendBtn->setMinimumWidth(80);
    sendCtrlLayout->addWidget(m_sendBtn);
    sendLayout->addLayout(sendCtrlLayout);

    mainLayout->addWidget(sendGroup);

    // ========== 4. 状态栏 ==========
    m_statusLabel = new QLabel("串口已关闭");
    statusBar()->addWidget(m_statusLabel);

    // ---------- 初始化串口对象 ----------
    m_serial = new QSerialPort(this);

    // ---------- 扫描可用串口 ----------
    populatePorts();

    // ---------- 连接信号与槽 ----------
    connect(m_openBtn, &QPushButton::clicked, this, &MainWindow::openClosePort);
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::sendData);
    connect(m_clearReceiveBtn, &QPushButton::clicked, this, &MainWindow::clearReceive);
    connect(m_clearSendBtn, &QPushButton::clicked, this, &MainWindow::clearSend);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::receiveData);
}

// 扫描系统上所有可用串口，填入端口下拉框
void MainWindow::populatePorts()
{
    m_portCombo->clear();

    // 如果串口已打开，不刷新
    if (m_serial && m_serial->isOpen()) {
        m_portCombo->addItem(m_serial->portName());
        return;
    }

    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &info : ports) {
        QString desc = info.portName();
        if (!info.description().isEmpty())
            desc += " - " + info.description();
        m_portCombo->addItem(desc, info.portName());
    }

    // 没有串口时显示提示
    if (m_portCombo->count() == 0) {
        m_portCombo->addItem("无可用串口");
    }
}

// 打开/关闭串口
void MainWindow::openClosePort()
{
    if (m_serial->isOpen()) {
        // ---- 关闭串口 ----
        m_serial->close();
        m_openBtn->setText("打开串口");
        m_statusLabel->setText("串口已关闭");
        // 恢复控件可编辑
        m_portCombo->setEnabled(true);
        m_baudCombo->setEnabled(true);
        m_dataBitsCombo->setEnabled(true);
        m_stopBitsCombo->setEnabled(true);
        m_parityCombo->setEnabled(true);
        return;
    }

    // ---- 打开串口 ----
    // 从下拉框获取实际端口名
    QString portName = m_portCombo->currentData().toString();
    if (portName.isEmpty())
        portName = m_portCombo->currentText();

    if (portName.isEmpty() || portName == "无可用串口") {
        m_statusLabel->setText("错误: 无可用串口");
        return;
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(m_baudCombo->currentText().toInt());

    // 数据位
    switch (m_dataBitsCombo->currentText().toInt()) {
    case 5:  m_serial->setDataBits(QSerialPort::Data5); break;
    case 6:  m_serial->setDataBits(QSerialPort::Data6); break;
    case 7:  m_serial->setDataBits(QSerialPort::Data7); break;
    case 8:  m_serial->setDataBits(QSerialPort::Data8); break;
    default: m_serial->setDataBits(QSerialPort::Data8); break;
    }

    // 停止位
    if (m_stopBitsCombo->currentText() == "1.5")
        m_serial->setStopBits(QSerialPort::OneAndHalfStop);
    else if (m_stopBitsCombo->currentText() == "2")
        m_serial->setStopBits(QSerialPort::TwoStop);
    else
        m_serial->setStopBits(QSerialPort::OneStop);

    // 校验位
    QString parity = m_parityCombo->currentText();
    if (parity == "None")      m_serial->setParity(QSerialPort::NoParity);
    else if (parity == "Even") m_serial->setParity(QSerialPort::EvenParity);
    else if (parity == "Odd")  m_serial->setParity(QSerialPort::OddParity);
    else if (parity == "Mark") m_serial->setParity(QSerialPort::MarkParity);
    else if (parity == "Space")m_serial->setParity(QSerialPort::SpaceParity);

    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_openBtn->setText("关闭串口");
        m_statusLabel->setText(QString("已打开: %1 | %2 %3%4%5")
                                  .arg(portName,
                                       m_baudCombo->currentText(),
                                       m_dataBitsCombo->currentText(),
                                       m_stopBitsCombo->currentText(),
                                       parity.left(1)));
        // 锁定参数，防止打开状态下修改
        m_portCombo->setEnabled(false);
        m_baudCombo->setEnabled(false);
        m_dataBitsCombo->setEnabled(false);
        m_stopBitsCombo->setEnabled(false);
        m_parityCombo->setEnabled(false);
    } else {
        m_statusLabel->setText("错误: 打开失败 - " + m_serial->errorString());
    }
}

// 发送数据
void MainWindow::sendData()
{
    if (!m_serial->isOpen()) {
        m_statusLabel->setText("错误: 串口未打开");
        return;
    }

    QString text = m_sendEdit->toPlainText();
    if (text.isEmpty()) return;

    QByteArray data;
    if (m_hexSendCheck->isChecked()) {
        // Hex 模式: 将 "01 02 A3" 这样的字符串转为字节数组
        QString hex = text.simplified().remove(' ');
        data = QByteArray::fromHex(hex.toUtf8());
    } else {
        // 文本模式: 直接发送 UTF-8 编码
        data = text.toUtf8();
    }

    qint64 written = m_serial->write(data);
    if (written >= 0) {
        m_statusLabel->setText(QString("已发送 %1 字节").arg(written));
    } else {
        m_statusLabel->setText("发送失败");
    }
}

// 清空接收区
void MainWindow::clearReceive()
{
    m_receiveEdit->clear();
}

// 清空发送区
void MainWindow::clearSend()
{
    m_sendEdit->clear();
}

// 接收数据
void MainWindow::receiveData()
{
    QByteArray data = m_serial->readAll();
    if (data.isEmpty()) return;

    if (m_hexReceiveCheck->isChecked()) {
        // Hex 显示: 将字节转为大写十六进制，空格分隔
        QString hexStr;
        for (int i = 0; i < data.size(); ++i) {
            hexStr += QString::number(static_cast<uint8_t>(data[i]), 16)
                          .rightJustified(2, '0')
                          .toUpper();
            if (i < data.size() - 1)
                hexStr += ' ';
        }
        m_receiveEdit->append(hexStr);
    } else {
        // 文本显示
        m_receiveEdit->append(QString::fromUtf8(data));
    }
}
