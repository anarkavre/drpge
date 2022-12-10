set(SOURCE_FILES
	CGrid.cpp		CGrid.h
				CList.h
	CMap.cpp		CMap.h
				CNode.h
	doomrpg_data.c		doomrpg_data.h
				doomrpg_entities.h
	main.cpp)

if(WIN32)
	add_executable(drpge WIN32 ${SOURCE_FILES})
else()
	add_executable(drpg ${SOURCE_FILES})
endif()

target_link_libraries(drpge ${SDL2_LIBRARIES})