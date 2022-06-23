cmake_minimum_required (VERSION 3.8)

if(WIN32)
	set(CMAKE_PREFIX_PATH "C:/dev/vcpkg/installed/x64-windows/share")
	set(CMAKE_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

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

find_package(PNG REQUIRED)

find_package(JPEG REQUIRED)

include_directories("include")

file(GLOB SRC "src/*.cpp")

add_executable(${PROJECT_NAME} ${SUBSYSTEM} ${SRC})

target_link_libraries(${PROJECT_NAME} ocom oui PNG::PNG JPEG::JPEG)

target_include_directories(${PROJECT_NAME} PRIVATE ${JPEG_LIBRARIES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)