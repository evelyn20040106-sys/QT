/**
 * @file main.cpp
 * @brief 人脸识别考勤系统的入口文件，负责启动 Qt 应用程序
 * @author 开发团队
 * @date 2026-07-08
 */

#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QFile>

/**
 * @brief 应用程序入口函数
 *
 * 创建 QApplication 实例，设置应用名称和版本号，
 * 实例化主窗口并显示，进入 Qt 事件循环。
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 */
int main(int argc, char *argv[])
{
    // 创建 Qt 应用程序对象，管理全局资源
    QApplication a(argc, argv);
    // 设置应用程序名称（用于 QSettings 等）
    QApplication::setApplicationName("人脸识别考勤系统");
    // 设置应用程序版本号
    QApplication::setApplicationVersion("1.0.0");

    // Check for OpenCV runtime DLLs
    // (OpenCV's dlls need to be in PATH or app directory)
    // OpenCV 运行时 DLL 需位于 PATH 或应用程序目录中，否则程序可能无法启动

    // 创建主窗口实例
    MainWindow w;
    // 显示主窗口
    w.show();

    // 进入 Qt 事件循环，等待用户操作
    return QApplication::exec();
}
