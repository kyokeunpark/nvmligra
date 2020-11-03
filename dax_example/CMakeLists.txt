cmake_minimum_required(VERSION 3.1.0)
project(cpp_nvm_test)

set (CMAKE_CXX_STANDARD 17)
include_directories(/usr/local/include)
add_executable(nvmtest main.cpp)

target_link_libraries(nvmtest pmem pmemobj)
