cmake_minimum_required(VERSION 3.6)
project(start)

set(CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}) #在当前目录下查找模块

find_package(Qt5 REQUIRED COMPONENTS Core Widgets REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(SDL2 REQUIRED)
find_Package(OpenCV REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE AUTOUIC ON)



# 注意 ： 要想使用qt5_add_resources，必须先find_package到Qt的路径或者手动指定QT_DIR
set(QT_SOURCE_FILE ui.qrc) # 将资源文件放到工程目录下，该部分会由RCC编译器预编译为cpp文件
qt5_add_resources(QT_RESOURCES ${QT_SOURCE_FILE})

set(CPP_SOURCE main.cpp video.cpp audio.cpp media.cpp Titlebar.cpp videoPlayer.cpp) 

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${FFMPEG_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(player ${CPP_SOURCE} ${QT_RESOURCES})

target_link_libraries(player PRIVATE Qt5::Core Qt5::Widgets pthread ${OpenCV_LIBS} ${FFMPEG_LIBS} ${SDL2_LIBS})
