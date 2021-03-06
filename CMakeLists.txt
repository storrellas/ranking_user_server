cmake_minimum_required(VERSION 2.8)

project(ranking_user_server)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

# Add C++11 standard
string(CONCAT CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Get source files
file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
# Remove main
list(REMOVE_ITEM SRC_LIST ${PROJECT_SOURCE_DIR}/src/main.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SRC_LIST};./src/main.cpp)
target_link_libraries (${PROJECT_NAME} "boost_system")
target_link_libraries (${PROJECT_NAME} "boost_thread")

# Create test executable	
set(PROJECT_TEST_NAME ${PROJECT_NAME}_test)
add_executable(${PROJECT_TEST_NAME} ${SRC_LIST};./test/ranking_user_server_test.cpp )
target_link_libraries (${PROJECT_TEST_NAME} "boost_system")
target_link_libraries (${PROJECT_TEST_NAME} "boost_thread")
target_link_libraries (${PROJECT_TEST_NAME} "boost_unit_test_framework")
target_link_libraries (${PROJECT_TEST_NAME} "pthread")


ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK) 

enable_testing()
add_test(${PROJECT_TEST_NAME} bin/ranking_user_server_test )



