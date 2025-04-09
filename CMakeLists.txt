cmake_minimum_required(VERSION 3.16)
project(NodeBasedImageEditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Widgets OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Node.cpp
    src/Port.cpp
    src/Connection.cpp
    src/NodeGraphScene.cpp
    src/ImageViewerNode.cpp
    src/ImageLoaderNode.cpp
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
    ${OpenCV_LIBS}
)