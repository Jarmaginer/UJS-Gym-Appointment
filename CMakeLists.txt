cmake_minimum_required(VERSION 3.20)

project(main)

add_executable(main main.cpp)

find_package(CURL REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(main PRIVATE JsonCpp::JsonCpp)
target_link_libraries(main PRIVATE CURL::libcurl)