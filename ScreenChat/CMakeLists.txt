cmake_minimum_required(VERSION 3.0)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})
set(CMAKE_SHARED_LIBRARY_PREFIX "../")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".asi")

option(MAP "Generate .map file [ON/OFF]" OFF)
option(UPX "Execute UPX after build [ON/OFF]" ON)

ADD_DEFINITIONS(-DPROJECT_NAME_C="${PROJECT_NAME}")

ADD_DEFINITIONS(-DWIN32=1)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_NAME}_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/loader ${PROJECT_NAME}_LIST)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	add_definitions("-ffunction-sections -fdata-sections -w")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -ffast-math -static -fno-stack-protector")
	if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
		add_definitions("-fexec-charset=CP1251")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
	endif()
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel" OR "${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--strip-all")
	elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -g")
		if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -gcodeview -Wl,-pdb=${PROJECT_NAME}.pdb")
		endif()
	endif()
	if(MAP)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Map=${PROJECT_NAME}.map")
	endif(MAP)
endif()

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_LIST})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
)

target_link_libraries(${PROJECT_NAME} llmo SRDescent SRCursor SREvents render ${Boost_LIBRARIES} samp)
if(DXHOOK)
	target_link_libraries(${PROJECT_NAME} ProxyDX9)
endif()
if(DRAWHOOK)
	target_link_libraries(${PROJECT_NAME} DrawHook)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
	if (UPX)
		find_program(UPX_BIN upx)
		if(UPX_BIN)
			add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${UPX_BIN} -9 $<TARGET_FILE:${PROJECT_NAME}>)
		else(UPX_BIN)
			message("UPX enabled, but not found in $PATH")
		endif(UPX_BIN)
	endif(UPX)
endif()
