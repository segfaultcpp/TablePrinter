cmake_minimum_required(VERSION 3.20)
project(TablePrinter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(TablePrinter example.cpp)