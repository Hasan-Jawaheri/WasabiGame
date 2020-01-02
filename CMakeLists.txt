cmake_minimum_required(VERSION 3.15)

project(WasabiGame)
set(CMAKE_CXX_STANDARD 17)

# Helper function for VS filters
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

############# Wasabi ###############
if (NOT WASABI_ROOT)
	message(FATAL_ERROR "Could not find Wasabi library!")
else()
    set(Wasabi_LIBRARY_RELEASE "${WASABI_ROOT}/lib/Release/wasabi.lib")
    set(Wasabi_LIBRARY_DEBUG "${WASABI_ROOT}/lib/Debug/wasabi.lib")
    set(Wasabi_INCLUDE_DIR "${WASABI_ROOT}/include/")
	message(STATUS "Wasabi library: " ${Wasabi_LIBRARY_RELEASE})
	message(STATUS "Wasabi include: " ${Wasabi_INCLUDE_DIR})
endif()
#####################################

############# Vulkan ###############
find_package(Vulkan)
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Vulkan library: " ${Vulkan_LIBRARY})
	message(STATUS "Vulkan include: " ${Vulkan_INCLUDE_DIRS})
endif()
link_libraries(${Vulkan_LIBRARY})
include_directories(SYSTEM ${Vulkan_INCLUDE_DIRS})
#####################################

file(GLOB_RECURSE WASABI_GAME_SOURCES "src/WasabiGame/*.cpp")
file(GLOB_RECURSE WASABI_GAME_HEADERS "include/WasabiGame/*")

file(GLOB_RECURSE ROLL_THE_BALL_SOURCES "src/RollTheBall/*.cpp")
file(GLOB_RECURSE ROLL_THE_BALL_HEADERS "include/RollTheBall/*")

file(GLOB_RECURSE RTB_SERVER_SOURCES "src/RTBServer/*.cpp")
file(GLOB_RECURSE RTB_SERVER_HEADERS "include/RTBServer/*")
file(GLOB_RECURSE RTB_SERVER_APP_SOURCES "src/RTBServer/Bootstrapper.cpp")
list(REMOVE_ITEM RTB_SERVER_SOURCES ${RTB_SERVER_APP_SOURCES})

file(GLOB_RECURSE RTB_CLIENT_SOURCES "src/RTBClient/*.cpp")
file(GLOB_RECURSE RTB_CLIENT_HEADERS "include/RTBClient/*")
file(GLOB_RECURSE RTB_CLIENT_APP_SOURCES "src/RTBClient/Bootstrapper.cpp")
list(REMOVE_ITEM RTB_CLIENT_SOURCES ${RTB_CLIENT_APP_SOURCES})

file(GLOB_RECURSE RTB_TEST_SOURCES "src/RTBTest/*.cpp")

file(GLOB_RECURSE RPG_SOURCES "src/RPG/*.cpp")
file(GLOB_RECURSE RPG_HEADERS "include/RPG/*")

include_directories("include/")
include_directories(${Wasabi_INCLUDE_DIR})

######################## Wasabi Game Library ########################
assign_source_group(${WASABI_GAME_SOURCES} ${WASABI_GAME_HEADERS})
add_library(wasabi_game STATIC ${WASABI_GAME_SOURCES} ${WASABI_GAME_HEADERS})
#####################################################################

########################### Roll The Ball ###########################
assign_source_group(${ROLL_THE_BALL_SOURCES} ${ROLL_THE_BALL_HEADERS})
add_library(roll_the_ball STATIC ${ROLL_THE_BALL_SOURCES} ${ROLL_THE_BALL_HEADERS})
#####################################################################

################### Roll The Ball Client Library ####################
assign_source_group(${RTB_CLIENT_SOURCES} ${RTB_CLIENT_HEADERS})
add_library(rtb_client_lib STATIC ${RTB_CLIENT_SOURCES} ${RTB_CLIENT_HEADERS})
#####################################################################

################### Roll The Ball Server Library ####################
assign_source_group(${RTB_SERVER_SOURCES} ${RTB_SERVER_HEADERS})
add_library(rtb_server_lib STATIC ${RTB_SERVER_SOURCES} ${RTB_SERVER_HEADERS})
#####################################################################

######################## Roll The Ball Client ########################
assign_source_group(${RTB_CLIENT_APP_SOURCES})
if (WIN32)
    add_executable(rtb_client WIN32 ${RTB_CLIENT_APP_SOURCES})
else()
    add_executable(rtb_client ${RTB_CLIENT_APP_SOURCES})
endif()
target_link_libraries(rtb_client wasabi_game)
target_link_libraries(rtb_client roll_the_ball)
target_link_libraries(rtb_client rtb_client_lib)
target_link_libraries(rtb_client optimized ${Wasabi_LIBRARY_RELEASE})
target_link_libraries(rtb_client debug ${Wasabi_LIBRARY_DEBUG})
set_property(TARGET rtb_client PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#####################################################################

######################## Roll The Ball Server ########################
assign_source_group(${RTB_SERVER_APP_SOURCES})
if (WIN32)
    add_executable(rtb_server WIN32 ${RTB_SERVER_APP_SOURCES})
else()
    add_executable(rtb_server ${RTB_SERVER_APP_SOURCES})
endif()
target_link_libraries(rtb_server wasabi_game)
target_link_libraries(rtb_server roll_the_ball)
target_link_libraries(rtb_server rtb_server_lib)
target_link_libraries(rtb_server optimized ${Wasabi_LIBRARY_RELEASE})
target_link_libraries(rtb_server debug ${Wasabi_LIBRARY_DEBUG})
set_property(TARGET rtb_server PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#####################################################################

######################## Roll The Ball Test #########################
if (WIN32)
    add_executable(rtb_test WIN32 ${RTB_TEST_SOURCES})
else()
    add_executable(rtb_test ${RTB_TEST_SOURCES})
endif()
target_link_libraries(rtb_test wasabi_game)
target_link_libraries(rtb_test roll_the_ball)
target_link_libraries(rtb_test rtb_client_lib)
target_link_libraries(rtb_test rtb_server_lib)
target_link_libraries(rtb_test optimized ${Wasabi_LIBRARY_RELEASE})
target_link_libraries(rtb_test debug ${Wasabi_LIBRARY_DEBUG})
set_property(TARGET rtb_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#####################################################################

########################### RPG Game Sample ###########################
assign_source_group(${RPG_SOURCS} ${RPG_HEADERS})
if (WIN32)
    add_executable(rpg WIN32 ${RPG_SOURCS} ${RPG_HEADERS})
else()
    add_executable(rpg ${RPG_SOURCS} ${RPG_HEADERS})
endif()
target_link_libraries(rpg wasabi_game)
target_link_libraries(rpg optimized ${Wasabi_LIBRARY_RELEASE})
target_link_libraries(rpg debug ${Wasabi_LIBRARY_DEBUG})
set_property(TARGET rpg PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#####################################################################
