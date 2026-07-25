#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class Calculator : public QWidget
{
    Q_OBJECT
public:
    explicit Calculator(QWidget *parent = nullptr);
    ~Calculator();
        private slots:
                        void digitClicked();      // 处理数字
    void operatorClicked();   // 处理符号
    void equalClicked();      // 处理等号
    void ceClicked();         // 处理归零
    void backspaceClicked();  // 处理删除
private:

    QLineEdit *display;
    double leftOperand;
    QString pendingOperator;
    bool waitingForOperand;
    void calculate(double rightOperand, const QString &op);
    QPushButton* createButton(const QString &text, const char *member);
signals:
};

#endif // CALCULATOR_H
