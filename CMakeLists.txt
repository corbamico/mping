cmake_minimum_required(VERSION 3.27)

set(VCPKG_TARGET_TRIPLET x64-windows)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(demo)
add_executable(demo src/main.cpp)

find_package(asio CONFIG REQUIRED)
target_link_libraries(demo PRIVATE asio asio::asio)