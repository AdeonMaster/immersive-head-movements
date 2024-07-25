cmake_minimum_required(VERSION 3.21)

project(immersive-head-movements)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC
	"include/*.h"

	"src/*.h"
	"src/*.cpp"
)

add_library(${PROJECT_NAME} SHARED ${SRC})

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		SCRAT_EXPORT
		__G2A
)

target_include_directories(${PROJECT_NAME}
	INTERFACE
		"include/"
	PRIVATE
		"src/"
)

add_subdirectory(dependencies)