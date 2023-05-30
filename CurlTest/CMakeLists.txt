cmake_minimum_required(VERSION 3.15)
project(CurlTest)

find_package(CURL REQUIRED)

add_executable(${PROJECT_NAME} ../CurlTest.cpp ../NetworkClient.cpp)
target_link_libraries(${PROJECT_NAME} CURL::libcurl)