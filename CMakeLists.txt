cmake_minimum_required(VERSION 3.15)

project(WasabiGame)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(WasabiGameHelpers)

############# Wasabi ###############
if (NOT WASABI_ROOT)
	message(FATAL_ERROR "Missing -DWASABI_ROOT=<root-wasabi-dist>")
endif()
#####################################

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

#
# Build dependencies
#
set(DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
build_loguru("LOGURU_DIR" "${DEPENDENCIES_DIR}")

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

include_directories("include/")

######################## Wasabi Game Library ########################
assign_source_group(${WASABI_GAME_SOURCES} ${WASABI_GAME_HEADERS})
add_library(wasabi_game STATIC ${WASABI_GAME_SOURCES} ${WASABI_GAME_HEADERS})
target_include_directories(wasabi_game PRIVATE SYSTEM "${LOGURU_DIR}/")
link_target_to_wasabi(wasabi_game ${WASABI_ROOT})
#####################################################################

########################### Roll The Ball ###########################
assign_source_group(${ROLL_THE_BALL_SOURCES} ${ROLL_THE_BALL_HEADERS})
add_library(roll_the_ball STATIC ${ROLL_THE_BALL_SOURCES} ${ROLL_THE_BALL_HEADERS})
target_include_directories(roll_the_ball PRIVATE SYSTEM "${LOGURU_DIR}/")
link_target_to_wasabi(roll_the_ball ${WASABI_ROOT})
#####################################################################

################### Roll The Ball Client Library ####################
assign_source_group(${RTB_CLIENT_SOURCES} ${RTB_CLIENT_HEADERS})
add_library(rtb_client_lib STATIC ${RTB_CLIENT_SOURCES} ${RTB_CLIENT_HEADERS})
target_include_directories(rtb_client_lib PRIVATE SYSTEM "${LOGURU_DIR}/")
link_target_to_wasabi(rtb_client_lib ${WASABI_ROOT})
#####################################################################

################### Roll The Ball Server Library ####################
assign_source_group(${RTB_SERVER_SOURCES} ${RTB_SERVER_HEADERS})
add_library(rtb_server_lib STATIC ${RTB_SERVER_SOURCES} ${RTB_SERVER_HEADERS})
target_include_directories(rtb_server_lib PRIVATE SYSTEM "${LOGURU_DIR}/")
link_target_to_wasabi(rtb_server_lib ${WASABI_ROOT})
#####################################################################

######################## Roll The Ball Client ########################
assign_source_group(${RTB_CLIENT_APP_SOURCES})
if (WIN32)
    add_executable(rtb_client WIN32 ${RTB_CLIENT_APP_SOURCES})
else()
    add_executable(rtb_client ${RTB_CLIENT_APP_SOURCES})
endif()
target_link_libraries(rtb_client loguru)
target_link_libraries(rtb_client wasabi_game)
target_link_libraries(rtb_client roll_the_ball)
target_link_libraries(rtb_client rtb_client_lib)
target_include_directories(rtb_client PRIVATE SYSTEM "${LOGURU_DIR}/")
link_target_to_wasabi(rtb_client ${WASABI_ROOT})
set_property(TARGET rtb_client PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#####################################################################

######################## Roll The Ball Server ########################
assign_source_group(${RTB_SERVER_APP_SOURCES})
if (WIN32)
    add_executable(rtb_server WIN32 ${RTB_SERVER_APP_SOURCES})
else()
    add_executable(rtb_server ${RTB_SERVER_APP_SOURCES})
endif()
target_link_libraries(rtb_server loguru)
target_link_libraries(rtb_server wasabi_game)
target_link_libraries(rtb_server roll_the_ball)
target_link_libraries(rtb_server rtb_server_lib)
target_include_directories(rtb_server PRIVATE SYSTEM "${LOGURU_DIR}/")
link_target_to_wasabi(rtb_server ${WASABI_ROOT})
set_property(TARGET rtb_server PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#####################################################################

######################## Roll The Ball Test #########################
if (WIN32)
    add_executable(rtb_test WIN32 ${RTB_TEST_SOURCES})
else()
    add_executable(rtb_test ${RTB_TEST_SOURCES})
endif()
target_link_libraries(rtb_test loguru)
target_link_libraries(rtb_test wasabi_game)
target_link_libraries(rtb_test roll_the_ball)
target_link_libraries(rtb_test rtb_client_lib)
target_link_libraries(rtb_test rtb_server_lib)
target_include_directories(rtb_test PRIVATE SYSTEM "${LOGURU_DIR}/")
link_target_to_wasabi(rtb_test ${WASABI_ROOT})
set_property(TARGET rtb_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#####################################################################

