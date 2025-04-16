cmake_minimum_required(VERSION 3.14)
project(userver-samples-multipart_service CXX)

find_package(userver COMPONENTS core etcd REQUIRED)

add_executable(${PROJECT_NAME} service.cpp)
target_link_libraries(${PROJECT_NAME} userver::core userver::etcd)

userver_testsuite_add_simple()
