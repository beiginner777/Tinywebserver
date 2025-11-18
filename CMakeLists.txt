# 设置最小CMake版本
cmake_minimum_required(VERSION 3.10)
# 设置项目名称
project(Tinywebserver)
# 设置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
# 设置支持c++11标准
set(CMAKE_CXX_STANDARD 11)
# 设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
# 添加子目录
add_subdirectory(src)