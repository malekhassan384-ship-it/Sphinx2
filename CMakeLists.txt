cmake_minimum_required(VERSION 3.16)
project(Sphinx2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Sphinx2
    main.cpp
    MainWindow.cpp
    VaultEngine.h
)

target_link_libraries(Sphinx2 PRIVATE Qt6::Widgets)
