#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("SmartNote");
    QApplication::setApplicationName("SmartNote");
    QApplication::setApplicationVersion("1.0.0");
    MainWindow w;
    w.show();
    return a.exec();
}
