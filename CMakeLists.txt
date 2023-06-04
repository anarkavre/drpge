cmake_minimum_required(VERSION 3.7)

project(drpge)

if(MSVC)
	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

find_package(SDL2 REQUIRED)

add_subdirectory(src)