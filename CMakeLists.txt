cmake_minimum_required(VERSION 3.5)
project(UDPChat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui Network Sql)

add_executable(udp-chat
    src/main.cpp
    src/AppConfig.cpp
    src/DatabaseLogger.cpp
    src/NetworkManager.cpp
    src/MainWindow.cpp
)

target_include_directories(udp-chat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(udp-chat
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::Sql
)