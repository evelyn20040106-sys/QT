cmake_minimum_required(VERSION 3.16)
project(FaceRecognition LANGUAGES CXX)


#人脸识别考勤管理系统

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt path
set(CMAKE_PREFIX_PATH "E:/Qt/6.11.1/mingw_64")

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# Find OpenCV
set(OpenCV_DIR "E:/opencv")
find_package(OpenCV REQUIRED COMPONENTS core imgproc objdetect videoio imgcodecs highgui face)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    databasemanager.cpp
    facerecognizer.cpp
    registerdialog.cpp
    employeedialog.cpp
    statisticdialog.cpp
)

set(HEADERS
    mainwindow.h
    databasemanager.h
    facerecognizer.h
    registerdialog.h
    employeedialog.h
    statisticdialog.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Sql
    ${OpenCV_LIBS}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)
