cmake_minimum_required(VERSION 3.5)

project(tcp-speed-asio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SANITIZER "-g3 -O0 -fno-inline -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER}")
# add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING=1)

find_package(Boost REQUIRED COMPONENTS coroutine)
find_package(Threads REQUIRED)

add_executable(server server.cpp)
target_link_libraries(server PUBLIC "${Boost_LIBRARIES}" Threads::Threads)

add_executable(client client.cpp)
target_link_libraries(client PUBLIC "${Boost_LIBRARIES}" Threads::Threads)
