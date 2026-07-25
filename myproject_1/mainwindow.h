#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QThread>
#include "CopyWorker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //禁止隐式转换
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void onSelectSource();
    void onSelectTarget();
    void onStartCopy();
    void onCopyProgress(qint64 bytesCopied, qint64 totalBytes);
    void onCopyFinished(bool success, const QString &errorMsg = "");

private:
    void setupUi();
    void updateStartButtonState();
    QLabel *lblSource;
    QLineEdit *editSource;
    QPushButton *btnSource;

    QLabel *lblTarget;
    QLineEdit *editTarget;
    QPushButton *btnTarget;

    QProgressBar *progressBar;
    QPushButton *btnStart;
    QLabel *lblStatus;

    QThread *workerThread = nullptr;//子线程对象
    copyworker  *worker = nullptr; //干活的工人

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
