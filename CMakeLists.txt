cmake_minimum_required(VERSION 3.10)
project(ConsoleChat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
add_definitions(-D_WIN32_WINNT=0x0A00)


add_executable(client client.cpp)
add_executable(server server.cpp)

target_include_directories(client PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})


target_link_libraries(client PRIVATE 
    Boost::system
)

target_link_libraries(server PRIVATE  
    Boost::system
)
