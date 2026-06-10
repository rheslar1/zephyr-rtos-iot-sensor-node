cmake_minimum_required(VERSION 3.16)
project(zephyr_rtos_iot_sensor_node LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(sensor_node_core
  src/SensorNode.cpp
)
target_include_directories(sensor_node_core PUBLIC include)
target_compile_options(sensor_node_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sensor_node_core)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

add_executable(sensor_node_tests tests/SensorNodeTests.cpp)
target_link_libraries(sensor_node_tests PRIVATE sensor_node_core)
target_compile_options(sensor_node_tests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME sensor_node_tests COMMAND sensor_node_tests)
