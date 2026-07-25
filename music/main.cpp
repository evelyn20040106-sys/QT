/**
 * @file main.cpp
 * @brief 音乐播放器程序入口
 *
 * 创建 QApplication 并启动主窗口事件循环。
 */

#include <QApplication>  // Qt 应用程序类，管理 GUI 应用的生命周期
#include "mainwindow.h"  // 主窗口头文件

/**
 * @brief 程序主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 应用程序退出码
 *
 * 初始化 Qt 应用程序环境，创建并显示主窗口，进入事件循环。
 */
int main(int argc, char *argv[])
{
    // 创建 Qt 应用程序对象，管理全局资源
    QApplication app(argc, argv);

    // 设置应用程序名称，用于窗口标题和系统设置
    app.setApplicationName("MusicPlayer");

    // 设置应用程序版本号
    app.setApplicationVersion("1.0");

    // 创建主窗口实例
    MainWindow window;

    // 设置窗口初始大小：宽 800px，高 600px
    window.resize(800, 600);

    // 显示窗口（默认是隐藏的）
    window.show();

    // 进入事件循环，等待用户操作，直到窗口关闭才返回
    return app.exec();
}
