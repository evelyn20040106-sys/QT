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

void MainWindow::setupUI()
{
    setWindowTitle("串口调试");
    resize(800, 600);

    //=====主布局区=====
    auto *centralwidget = new QWidget(this);
    setCentralWidget(centralwidget);
    auto *mainlayout = new QVBoxLayout(centralwidget);
    mainlayout->setContentsMargins(10, 10, 10, 10);

    //======串口配置区======
    auto *configgroup = new QGroupBox("串口配置");
    auto *configlayout = new QGridLayout(configgroup);

    //端口号
    configlayout->addWidget(new QLabel("端口号："), 0, 0);
    m_port = new QComboBox;
    m_port->setMinimumWidth(120);
    configlayout->addWidget(m_port, 0, 1);

    //波特率
    configlayout->addWidget(new QLabel("波特率："), 0, 2);
    m_baud = new QComboBox;
    m_baud->addItems({"9600", "19200", "38400", "57600", "115200", "20400"});
    m_baud->setCurrentText("115200");
    configlayout->addWidget(m_baud, 0, 3);

    //数据位
    configlayout->addWidget(new QLabel("数据位："), 0, 4);
    m_data = new QComboBox;
    m_data->addItems({"5", "6", "7", "8"});
    m_data->setCurrentText("8");
    configlayout->addWidget(m_data, 0, 5);

    //停止位
    configlayout->addWidget(new QLabel("停止位："), 0, 6);
    m_stop = new QComboBox;
    m_stop->addItems({"1", "1.5", "2"});
    m_stop->setCurrentText("1");
    configlayout->addWidget(m_stop, 0, 7);

    //校验位
    configlayout->addWidget(new QLabel("校验位："), 0, 8);
    m_parity = new QComboBox;
    m_parity->addItems({"none", "even", "odd", "mark", "space"});
    configlayout->addWidget(m_parity, 0, 9);

    //打开关闭按钮
    m_openbtn = new QPushButton("打开串口");
    m_openbtn->setMinimumWidth(90);
    configlayout->addWidget(m_openbtn, 0, 10);

    mainlayout->addWidget(configgroup);

    //=====接收区=====
    auto *receivegroup = new QGroupBox("接收区");
    auto *receivelayout = new QVBoxLayout(receivegroup);

    m_receiveedit = new QTextEdit;
    m_receiveedit->setReadOnly(true);
    m_receiveedit->setPlaceholderText("请看这里....");
    receivelayout->addWidget(m_receiveedit);

    auto *receivectrlayout = new QHBoxLayout;
    m_hexReceivecheck = new QCheckBox("hex 显示");
    receivectrlayout->addWidget(m_hexReceivecheck);
    receivectrlayout->addStretch();
    m_clearreceivebtn = new QPushButton("清空接收");
    receivectrlayout->addWidget(m_clearreceivebtn);
    receivelayout->addLayout(receivectrlayout);

    mainlayout->addWidget(receivegroup, 1);

    //发送区
    auto *sendgroup = new QGroupBox("发送区");
    auto *sendlayout = new QHBoxLayout(sendgroup);

    m_sendedit = new QTextEdit;
    m_sendedit->setPlaceholderText("输入要发送的数据...");
    m_sendedit->setMaximumHeight(120);
    sendlayout->addWidget(m_sendedit);

    auto *sendctrlayout = new QHBoxLayout;
    m_hexsendcheck = new QCheckBox("hex 发送");
    sendctrlayout->addWidget(m_hexsendcheck);
    sendctrlayout->addStretch();
    m_clearsendbtn = new QPushButton("清空发送");
    sendctrlayout->addWidget(m_clearsendbtn);
    m_sendbtn = new QPushButton("发送");
    m_sendbtn->setMinimumWidth(80);
    sendctrlayout->addWidget(m_sendbtn);
    sendlayout->addLayout(sendctrlayout);

    mainlayout->addWidget(sendgroup);

    //=====状态栏=====
    m_statuslabel = new QLabel("串口已关闭");
    statusBar()->addWidget(m_statuslabel);

    //=====初始化串口对象=====
    m_serial = new QSerialPort(this);
    //扫描可用串口
    populatePorts();

    connect(m_openbtn, &QPushButton::clicked, this, &MainWindow::openClosePort);
    connect(m_sendbtn, &QPushButton::clicked, this, &MainWindow::sendData);
    connect(m_clearreceivebtn, &QPushButton::clicked, this, &MainWindow::clearReceive);
    connect(m_clearsendbtn, &QPushButton::clicked, this, &MainWindow::clearSend);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::receiveData);
}

void MainWindow::populatePorts()
{
    m_port->clear();

    if(m_serial && m_serial->isOpen())
    {
        m_port->addItem(m_serial->portName());
        return ;
    }
    //获取当前计算机串口
    const auto ports = QSerialPortInfo::availablePorts();
    for(const auto &info : ports)
    {
        QString desc = info.portName();
        if(!info.description().isEmpty())
            desc += " - " + info.description();
        m_port->addItem(desc, info.portName());
    }

    if(m_port->count() == 0)
    {
        m_port->addItem("无可用串口");
    }
}

void MainWindow::openClosePort()
{
    if(m_serial->isOpen())
    {
        m_serial->close();
        m_openbtn->setText("打开串口");
        m_statuslabel->setText("串口已关闭");
        m_port->setEnabled(true);
        m_baud->setEnabled(true);
        m_data->setEnabled(true);
        m_stop->setEnabled(true);
        m_parity->setEnabled(true);
        return ;
    }
        //如果用户数据为空（比如没有设置过用户数据），
        // 就退一步直接用下拉框当前显示的文本
    QString portName = m_port->currentData().toString();
    if (portName.isEmpty())
        portName = m_port->currentText();

    if (portName.isEmpty() || portName == "无可用串口") {
        m_statuslabel->setText("错误: 无可用串口");
        return;
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(m_baud->currentText().toInt());

    // 数据位
    switch (m_data->currentText().toInt()) {
    case 5:  m_serial->setDataBits(QSerialPort::Data5); break;
    case 6:  m_serial->setDataBits(QSerialPort::Data6); break;
    case 7:  m_serial->setDataBits(QSerialPort::Data7); break;
    case 8:  m_serial->setDataBits(QSerialPort::Data8); break;
    default: m_serial->setDataBits(QSerialPort::Data8); break;
    }

    // 停止位
    if (m_stop->currentText() == "1.5")
        m_serial->setStopBits(QSerialPort::OneAndHalfStop);
    else if (m_stop->currentText() == "2")
        m_serial->setStopBits(QSerialPort::TwoStop);
    else
        m_serial->setStopBits(QSerialPort::OneStop);

    // 校验位
    QString parity = m_parity->currentText();
    if (parity == "None")      m_serial->setParity(QSerialPort::NoParity);
    else if (parity == "Even") m_serial->setParity(QSerialPort::EvenParity);
    else if (parity == "Odd")  m_serial->setParity(QSerialPort::OddParity);
    else if (parity == "Mark") m_serial->setParity(QSerialPort::MarkParity);
    else if (parity == "Space")m_serial->setParity(QSerialPort::SpaceParity);

    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_openbtn->setText("关闭串口");
        m_statuslabel->setText(QString("已打开: %1 | %2 %3%4%5")
                                   .arg(portName,
                                        m_baud->currentText(),
                                        m_data->currentText(),
                                        m_stop->currentText(),
                                        parity.left(1)
                                        )
                               );

        // 锁定参数，防止打开状态下修改
        m_port->setEnabled(false);
        m_baud->setEnabled(false);
        m_data->setEnabled(false);
        m_stop->setEnabled(false);
        m_parity->setEnabled(false);
    } else {
        m_statuslabel->setText("错误: 打开失败 - " + m_serial->errorString());
    }
}

void MainWindow::sendData()
{
    if(!m_serial->isOpen())
    {
        m_statuslabel->setText("错误：串口未打开");
        return ;
    }

    QString text = m_sendedit->toPlainText();
    if(text.isEmpty())
    {
        return ;
    }

    QByteArray data;

    //根据是否勾选"十六进制发送"选项，把输入框里的文字转成要发送的字节数据。
    if(m_hexsendcheck->isChecked())
    {
        QString hex = text.simplified().remove(' ');
        data = QByteArray::fromHex(hex.toUtf8());
    }
    else
    {
        data = text.toUtf8();
    }

    qint64 written = m_serial->write(data);
    if(written > 0)
    {
        m_statuslabel->setText(QString("已发送 %1 个字节").arg(written));
    }
    else
    {
        m_statuslabel->setText("发送失败");
    }
}

void MainWindow::clearReceive()
{
    m_receiveedit->clear();
}

void MainWindow::clearSend()
{
    m_sendedit->clear();
}

//处理接受数据
void MainWindow::receiveData()
{
    QByteArray data = m_serial->readAll();
    if(data.isEmpty())
        return ;

    //判断"十六进制接收"复选框有没有被勾上。
    if(m_hexReceivecheck->isChecked())
    {
        QString hexstr;
        for(int i = 0; i < data.size(); i++)
        {
            /**
             * static_cast<uint8_t> → 转成无符号整数（0~255）
             * QString::number(..., 16) → 转成十六进制字符串，比如 255 变成 "ff"
             * .rightJustified(2, '0') → 不够两位补零，比如 "f" 变成 "0f"
             * .toUpper() → 转大写，"0f" 变成 "0F"
             */
            hexstr += QString::number(static_cast<uint8_t>(data[i]), 16).rightJustified(2, '0').toUpper();
            if(i < data.size() - 1)
            {
                hexstr = ' ';
            }
        }
        m_receiveedit->append(hexstr);
    }
    else
    {
        m_receiveedit->append(QString::fromUtf8(data));
    }
}

































