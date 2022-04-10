cmake_minimum_required(VERSION 3.16)
project(1039I)
set(CMAKE_CXX_STANDARD 17)
include_directories(npl)

add_executable(main src/main.cpp)
add_executable(simpleServer src/simpleServer.cpp)
add_executable(simpleClient src/simpleClient.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
