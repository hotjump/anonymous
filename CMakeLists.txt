# CMake最低版本号要求
cmake_minimum_required (VERSION 2.6)

# 项目信息
project (anonymous)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 添加index和data_file目录下的文件
aux_source_directory(./index DIR_SRCS)

SET(CMAKE_CXX_FLAGS "-std=c++11 -g -ggdb -Wno-deprecated-declarations ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/common/hash
    ${PROJECT_SOURCE_DIR}/common/lz4/lib
    ${PROJECT_SOURCE_DIR}/index
    ${PROJECT_SOURCE_DIR}/data_file
)

add_subdirectory(index)

# 指定生成目标
add_executable(test test.cpp ${PROJECT_SOURCE_DIR}/data_file/DataFile.cpp)

# 添加链接库
target_link_libraries(test index)