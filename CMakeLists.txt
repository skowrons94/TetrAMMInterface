cmake_minimum_required(VERSION 3.0)
project(TetrAMMInterface VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Boost COMPONENTS thread system REQUIRED)

include_directories(
  ${HEADER_DIR}
)

add_executable(TetrAMMInterface
  ${SRC_DIR}/main.cc
  ${SRC_DIR}/TetrAMMInterface.cc
  ${SRC_DIR}/GraphiteConnector.cpp
  ${SRC_DIR}/GraphiteReporter.cpp
)

target_link_libraries(TetrAMMInterface PRIVATE ${Boost_LIBRARIES})