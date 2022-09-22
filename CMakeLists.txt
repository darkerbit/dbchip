cmake_minimum_required(VERSION 3.22)
project(dbchip C)

set(CMAKE_C_STANDARD 17)

add_executable(dbchip src/main.c src/vm.c src/vm.h src/render.c src/render.h src/timing.h src/decode.c src/input.c src/input.h)

if(WIN32)
	# TODO
else()
	target_sources(dbchip PRIVATE src/timing_linux.c)
endif()

# SDL2 Dependency
if (DEFINED SDL2_INCLUDE_DIRS AND DEFINED SDL2_LIBRARIES)
	message(STATUS "using pre-defined SDL2 variables SDL2_INCLUDE_DIRS and SDL2_LIBRARIES")
	target_include_directories(dbchip PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
	target_link_libraries(dbchip PUBLIC ${SDL2_LIBRARIES})
else()
	# Only try to autodetect if both SDL2 variables aren't explicitly set
	find_package(SDL2 CONFIG)
	if (TARGET SDL2::SDL2)
		message(STATUS "using TARGET SDL2::SDL2")
		target_link_libraries(dbchip PUBLIC SDL2::SDL2)
	elseif (TARGET SDL2)
		message(STATUS "using TARGET SDL2")
		target_link_libraries(dbchip PUBLIC SDL2)
	else()
		message(STATUS "no TARGET SDL2::SDL2, or SDL2, using variables")
		target_include_directories(dbchip PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
		target_link_libraries(dbchip PUBLIC ${SDL2_LIBRARIES})
	endif()
endif()
