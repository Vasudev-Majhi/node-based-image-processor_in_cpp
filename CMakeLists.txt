cmake_minimum_required(VERSION 3.10)
project(NodeBasedImageEditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)  # Critical for Qt projects
find_package(OpenCV REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(NodeBasedImageEditor
    src/main.cpp
    src/node.h
    src/node.cpp
    src/socket.h
    src/socket.cpp
    src/connection.h
    src/connection.cpp
    src/node_graph.h
    src/node_graph.cpp
    src/node_editor.h
    src/node_editor.cpp
    # src/test_node.h
    # src/test_node.cpp
)

target_link_libraries(NodeBasedImageEditor
    Qt5::Core
    Qt5::Widgets
    ${OpenCV_LIBS}
)