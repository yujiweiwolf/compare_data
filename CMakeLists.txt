# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.16)
project(compare_data)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
SET(CMAKE_BUILD_TYPE "Debug")  # Release Debug
#add_compile_options(-std=c++17 -Wall -O2)
add_compile_options(-std=c++17)

message("*****************************************")
find_package(X REQUIRED)
include_directories(${X_INCLUDE_DIRS})
link_directories(${X_LIBRARY_DIRS})

find_package(CORAL REQUIRED)
include_directories(${CORAL_INCLUDE_DIRS})
link_directories(${CORAL_LIBRARY_DIRS})

find_package(FEEDER REQUIRED)
include_directories(${FEEDER_INCLUDE_DIRS})
link_directories(${FEEDER_LIBRARY_DIRS})
message("*****************************************")

#aux_source_directory (./src/compare_one COMPAREONE)
#add_executable(compare_one ${COMPAREONE})
#target_link_libraries(compare_one ${FEEDER_LIBRARIES} ${CORAL_LIBRARIES} ${X_LIBRARIES} boost_program_options protobuf protobuf-lite)

add_executable(code_index src/code_index/code_index.cc src/code_index/test_code_index.cc)
target_link_libraries(code_index gtest gtest_main x stdc++fs yaml-cpp clickhouse-cpp-lib-static boost_date_time boost_filesystem boost_regex boost_system  boost_chrono boost_log boost_program_options boost_thread boost_iostreams z protobuf protobuf-lite sodium zmq ssl crypto iconv pthread dl)

SET(FEEDER compare_all_instrument)
aux_source_directory (./src/compare_all COMPAREONE_ALL)
add_executable(${FEEDER} ${COMPAREONE_ALL})
target_link_libraries(${FEEDER} ${FEEDER_LIBRARIES} ${CORAL_LIBRARIES} ${X_LIBRARIES} boost_program_options protobuf protobuf-lite)

SET(ROOT_DIR "/home/work/sys/lib")
FILE(GLOB COMMON_LIB_NAME "${ROOT_DIR}/common_lib/lib/*so*")
FILE(COPY ${COMMON_LIB_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
FILE(COPY Dockerfile image.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(image COMMAND sh image.sh ${FEEDER} DEPENDS ${FEEDER})


