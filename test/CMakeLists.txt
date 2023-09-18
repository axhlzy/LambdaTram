cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 17)

project(test)

file(GLOB TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${TEST_SRC_FILES})

if(ANDROID)
    find_library(log-lib log)
    target_link_libraries(${PROJECT_NAME} ${log-lib})
endif()
