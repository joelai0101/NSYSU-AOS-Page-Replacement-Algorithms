cmake_minimum_required(VERSION 3.24.2)

project(aos_page_replacement)

set (CMAKE_CXX_STANDARD 11)

add_executable(main main.cpp referenceString/referenceString.cpp performanceReport/performanceReport.cpp pageReplacement/pageReplacement.cpp)

target_include_directories(main PUBLIC performanceReport)