cmake_minimum_required(VERSION 3.10)
SET(PROJECT_NAME ACL_tuner)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(REMOVE_CACHE=1
						CORE_COUNT=8
						DEBUG_LOG=1
						DEFAULT_CORE=4,5,6,7
						WARM_UP_CORE=0,1,2,3,4,5,6,7
                        )
                        
message(STATUS "Source directory : ${CMAKE_SOURCE_DIR}")
MESSAGE(STATUS "Binary directory : ${CMAKE_BINARY_DIR}")

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_SOURCE_DIR}/tuner/module/json)

include_directories(${CMAKE_SOURCE_DIR}/tuner/include)
include_directories(${CMAKE_SOURCE_DIR}/tuner/module)
include_directories(${CMAKE_SOURCE_DIR}/base)

link_directories(${CMAKE_SOURCE_DIR}/external)

add_executable(${PROJECT_NAME} main.cpp
        tuner/src/optimizer/random.cpp
        tuner/src/optimizer/optuna.cpp
        tuner/src/optimizer/sequncial.cpp
        tuner/src/tune_engine.cpp
        tuner/src/strategy_factory.cpp
        tuner/src/exec.cpp
        tuner/src/optimizer.cpp
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

target_link_libraries(${PROJ_NAME} nlohmann_json::nlohmann_json)
