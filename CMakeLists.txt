cmake_minimum_required(VERSION 3.19)
project(PythonAwaiterForCpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(INCLUDE_DIR include)

# packages
find_package(Python3 REQUIRED COMPONENTS Development)
find_package(Boost REQUIRED COMPONENTS system thread locale)

# lib
file(GLOB SRCS src/*.cpp)
add_library(pyawaiter STATIC ${SRCS})
target_include_directories(pyawaiter PRIVATE ${INCLUDE_DIR})
target_include_directories(pyawaiter PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(pyawaiter PUBLIC
    Boost::system
    Boost::thread
    ${Python3_LIBRARIES}
)

# executables
add_executable(example example/main.cpp)
target_include_directories(example PRIVATE ${INCLUDE_DIR})
target_include_directories(example PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(example PRIVATE
    pyawaiter
)
