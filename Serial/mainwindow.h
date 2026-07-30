#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
class QComboBox;
class QTextEdit;
class QPushButton;
class QLabel;
class QCheckBox;
class QSpinBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void openClosePort();
    void sendData();
    void clearReceive();
    void clearSend();
    void receiveData();

private:
    void setupUI();
    void populatePorts();

    // 串口相关
    QSerialPort *m_serial = nullptr;

    // 串口配置控件
    QComboBox *m_portCombo = nullptr;
    QComboBox *m_baudCombo = nullptr;
    QComboBox *m_dataBitsCombo = nullptr;
    QComboBox *m_stopBitsCombo = nullptr;
    QComboBox *m_parityCombo = nullptr;

    // 操作按钮
    QPushButton *m_openBtn = nullptr;
    QPushButton *m_sendBtn = nullptr;
    QPushButton *m_clearReceiveBtn = nullptr;
    QPushButton *m_clearSendBtn = nullptr;

    // 数据显示区
    QTextEdit *m_receiveEdit = nullptr;
    QTextEdit *m_sendEdit = nullptr;

    // 选项
    QCheckBox *m_hexReceiveCheck = nullptr;
    QCheckBox *m_hexSendCheck = nullptr;

    // 状态栏标签
    QLabel *m_statusLabel = nullptr;
};

#endif // MAINWINDOW_H
