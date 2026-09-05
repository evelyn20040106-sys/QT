#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName("影视拷贝工具");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("DaShiXiong Studio");

    MainWindow w;
    w.setWindowTitle("影视胶片备份工具 v1.0");
    w.resize(720, 420);
    w.show();
    return a.exec();
}
