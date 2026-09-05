#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

Calculator::Calculator(QWidget *parent)
    : QWidget(parent), leftOperand(0.0), waitingForOperand(true)
{
    setWindowTitle("冯乃旺的计算器");

    //设置主布局
    QGridLayout *mainlayout = new QGridLayout(this);

    //顶部显示框
    display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);

    QFont font = display->font();
    font.setPointSize(16);
    display->setFont(font);
    display->setMinimumHeight(50);
    mainlayout->addWidget(display, 0, 0, 1, 5);

    //数字
    QPushButton *btn1 = createButton("1", SLOT(digitClicked()));
    QPushButton *btn2 = createButton("2", SLOT(digitClicked()));
    QPushButton *btn3 = createButton("3", SLOT(digitClicked()));
    QPushButton *btn4 = createButton("4", SLOT(digitClicked()));
    QPushButton *btn5 = createButton("5", SLOT(digitClicked()));
    QPushButton *btn6 = createButton("6", SLOT(digitClicked()));
    QPushButton *btn7 = createButton("7", SLOT(digitClicked()));
    QPushButton *btn8 = createButton("8", SLOT(digitClicked()));
    QPushButton *btn9 = createButton("9", SLOT(digitClicked()));
    QPushButton *btn0 = createButton("0", SLOT(digitClicked()));
    QPushButton *btnDot = createButton(".", SLOT(digitClicked()));

    //符号
    QPushButton *btnPlus = createButton("+", SLOT(operatorClicked()));
    QPushButton *btnMinus = createButton("-", SLOT(operatorClicked()));
    QPushButton *btnMult = createButton("*", SLOT(operatorClicked()));
    QPushButton *btnDiv = createButton("/", SLOT(operatorClicked()));

    //操作
    QPushButton *btnEqual = createButton("=", SLOT(equalClicked()));
    QPushButton *btnCE = createButton("CE", SLOT(ceClicked()));
    QPushButton *btnBack = createButton("<-", SLOT(backspaceClicked()));

    mainlayout->addWidget(btn1, 1, 0);
    mainlayout->addWidget(btn2, 1, 1);
    mainlayout->addWidget(btn3, 1, 2);
    mainlayout->addWidget(btnPlus, 1, 3, 2, 1);
    btnPlus->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainlayout->addWidget(btnCE, 1, 4);

    mainlayout->addWidget(btn4, 2, 0);
    mainlayout->addWidget(btn5, 2, 1);
    mainlayout->addWidget(btn6, 2, 2);
    mainlayout->addWidget(btnBack, 2, 4);


    mainlayout->addWidget(btn7, 3, 0);
    mainlayout->addWidget(btn8, 3, 1);
    mainlayout->addWidget(btn9, 3, 2);
    mainlayout->addWidget(btnMinus, 3, 3);

    mainlayout->addWidget(btnEqual, 3, 4, 2, 1);
    btnEqual->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainlayout->addWidget(btnDot, 4, 0);
    mainlayout->addWidget(btn0, 4, 1);
    mainlayout->addWidget(btnMult, 4, 2);
    mainlayout->addWidget(btnDiv, 4, 3);

    setLayout(mainlayout);
    setFixedSize(300, 350);
}

//创建按钮，连接信号
QPushButton* Calculator::createButton(const QString &text, const char *member) {
    QPushButton *button = new QPushButton(text);
    button->setMinimumSize(40, 40);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}

//处理数字点击
void Calculator::digitClicked() {

    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    QString digitValue = clickedButton->text();

    if (display->text() == "0" && digitValue != ".") {
        display->clear();
    }

    if (waitingForOperand) {
        display->clear();
        waitingForOperand = false;
    }

    display->setText(display->text() + digitValue);
}

//处理符号点击
void Calculator::operatorClicked() {
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    QString clickedOperator = clickedButton->text();
    double operand = display->text().toDouble();

    if (!pendingOperator.isEmpty()) {
        calculate(operand, pendingOperator);
        display->setText(QString::number(leftOperand));
    } else {
        leftOperand = operand;
    }

    pendingOperator = clickedOperator;

    waitingForOperand = true;
}

//处理=点击
void Calculator::equalClicked() {
    double operand = display->text().toDouble();

    if (!pendingOperator.isEmpty()) {
        calculate(operand, pendingOperator);
        pendingOperator.clear();
    } else {
        leftOperand = operand;
    }

    display->setText(QString::number(leftOperand));
    waitingForOperand = true;
}

//清0
void Calculator::ceClicked() {

    leftOperand = 0.0;
    pendingOperator.clear();

    display->setText("0");
    waitingForOperand = true;
}

//删除
void Calculator::backspaceClicked() {

    if (waitingForOperand) return;

    QString text = display->text();
    text.chop(1);

    if (text.isEmpty()) {
        text = "0";
        waitingForOperand = true;
    }
    display->setText(text);
}

//加减乘除
void Calculator::calculate(double rightOperand, const QString &op) {
    if (op == "+") {
        leftOperand += rightOperand;
    } else if (op == "-") {
        leftOperand -= rightOperand;
    } else if (op == "*") {
        leftOperand *= rightOperand;
    } else if (op == "/") {
        if (rightOperand != 0.0)
            leftOperand /= rightOperand;
        else
            leftOperand = 0.0;
    }
}

Calculator::~Calculator() {}














