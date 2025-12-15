cmake_minimum_required(VERSION 4.0)
project(7-Wonders)

set(CMAKE_CXX_STANDARD 14)

add_executable(7-Wonders main.cpp
        Controller.cpp
        Engine.cpp
        Model.cpp
        Utils.cpp
        View.cpp
)
