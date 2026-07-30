#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

QT_BEGIN_INCLUDE_NAMESPACE
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

    //串口
    QSerialPort *m_serial = nullptr;

    //串口配置
    QComboBox *m_port = nullptr;
    QComboBox *m_baud = nullptr;
    QComboBox *m_data = nullptr;
    QComboBox *m_stop = nullptr;
    QComboBox *m_parity = nullptr;

    //操作按钮
    QPushButton *m_openbtn = nullptr;
    QPushButton *m_sendbtn = nullptr;
    QPushButton *m_clearreceivebtn = nullptr;
    QPushButton *m_clearsendbtn = nullptr;

    //数据显示
    QTextEdit *m_receiveedit = nullptr;
    QTextEdit *m_sendedit = nullptr;

    //选项
    QCheckBox *m_hexReceivecheck = nullptr;
    QCheckBox *m_hexsendcheck = nullptr;

    //状态栏标签
    QLabel *m_statuslabel= nullptr;
};

#endif // MAINWINDOW_H
