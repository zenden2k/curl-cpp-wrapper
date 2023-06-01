cmake_minimum_required(VERSION 3.15)
project(CurlTest)

find_package(CURL REQUIRED)
find_package(GTest REQUIRED)
find_package(jsoncpp REQUIRED)

add_executable(${PROJECT_NAME} ../NetworkClientTest.cpp ../NetworkClient.cpp)
target_link_libraries(${PROJECT_NAME} CURL::libcurl gtest jsoncpp::jsoncpp)