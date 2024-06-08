cmake_minimum_required(VERSION 3.10)
SET(PROJECT_NAME ACL_tuner)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Source directory : ${CMAKE_SOURCE_DIR}")
MESSAGE(STATUS "Binary directory : ${CMAKE_BINARY_DIR}")

add_subdirectory(${CMAKE_SOURCE_DIR}/module/json)

include_directories(${CMAKE_SOURCE_DIR}/tuner/include)
include_directories(${CMAKE_SOURCE_DIR}/tuner/module)
include_directories(${CMAKE_SOURCE_DIR}/base)

link_directories(${CMAKE_SOURCE_DIR}/external)

add_executable(${PROJECT_NAME} main.cpp
        tuner/random.cpp
        tuner/optuna.cpp
        tuner/tune_engine.cpp
        tuner/strategy_factory.cpp
        base/utils/Utils.cpp 
        base/utils/GraphUtils.cpp 
        base/utils/CommonGraphOptions.cpp
)

target_link_libraries(${PROJECT_NAME}
        arm_compute_graph
        arm_compute
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/base
        ${CMAKE_SOURCE_DIR}/include)
