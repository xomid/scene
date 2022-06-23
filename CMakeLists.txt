cmake_minimum_required (VERSION 3.8)

project (scene)

set(SUBSYSTEM WIN32)

if(WIN32)
	set(GLOBAL_OUT_DIR "C:/Users/Omid/Documents/cmake/out")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${GLOBAL_OUT_DIR}/bin/${CMAKE_BUILD_TYPE}")
	set(LIBRARY_OUTPUT_PATH "${GLOBAL_OUT_DIR}/lib/${CMAKE_BUILD_TYPE}")
	
	link_directories("${GLOBAL_OUT_DIR}/lib/${CMAKE_BUILD_TYPE}")
	include_directories(${PROJECT_NAME} "${GLOBAL_OUT_DIR}/include/ocom")
	include_directories(${PROJECT_NAME} "${GLOBAL_OUT_DIR}/include/oui")
endif()

include_directories("include")

file(GLOB SRC "src/*.cpp")

add_executable(${PROJECT_NAME} ${SUBSYSTEM} ${SRC})

target_link_libraries(${PROJECT_NAME} ocom oui)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)