# cmake_minimum_required: 指定最低 CMake 版本，确保构建系统特性可用
cmake_minimum_required(VERSION 3.20)

# project: 定义项目名称、版本号和使用的编程语言（C++）
project(MusicPlayer VERSION 1.0.0 LANGUAGES CXX)

#播放器

# ------------------------------------------------------------
# C++ 标准配置
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)          # 使用 C++17 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制要求编译器支持 C++17
set(CMAKE_AUTOMOC ON)               # 自动调用 MOC（元对象编译器）处理 Q_OBJECT 宏
set(CMAKE_AUTORCC ON)               # 自动调用 RCC（资源编译器）
set(CMAKE_AUTOUIC ON)               # 自动调用 UIC（UI 编译器）

# find_package: 查找 Qt6 库，必须包含 Core、Gui、Widgets、Multimedia 模块
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

# add_executable: 将源文件编译成可执行文件 MusicPlayer.exe
add_executable(MusicPlayer
    main.cpp              # 程序入口
    mainwindow.h          # 主窗口头文件
    mainwindow.cpp        # 主窗口实现
    musicplayer.h         # 音乐播放器核心头文件
    musicplayer.cpp       # 音乐播放器核心实现
    musicscanner.h        # 音乐扫描器头文件
    musicscanner.cpp      # 音乐扫描器实现
    visualizerwidget.h    # 频谱动效控件头文件
    visualizerwidget.cpp  # 频谱动效控件实现
)

# target_link_libraries: 将可执行文件与 Qt6 库链接
target_link_libraries(MusicPlayer PRIVATE
    Qt6::Core       # Qt 核心模块（QObject、事件循环等）
    Qt6::Gui        # Qt 图形界面模块（QPainter、字体等）
    Qt6::Widgets    # Qt 窗口组件模块（按钮、布局等）
    Qt6::Multimedia # Qt 多媒体模块（音频播放等）
)
