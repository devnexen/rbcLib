project(rbctool)
cmake_minimum_required(VERSION 2.6)

set(RBC_LIBRARY rbcserial)
set(LDL "")

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(LDL "-ldl")
endif()

find_package(Boost COMPONENTS thread system REQUIRED)

message(STATUS ${Boost_LIBRARIES})
message(STATUS ${Boost_INCLUDE_DIR})

include_directories(${Boost_INCLUDE_DIR})

add_library(${RBC_LIBRARY} SHARED rbcserial.cpp)
target_link_libraries(${RBC_LIBRARY} ${Boost_LIBRARIES})

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${RBC_LIBRARY} ${Boost_LIBRARIES} ${LDL})
add_subdirectory(plugins)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)
