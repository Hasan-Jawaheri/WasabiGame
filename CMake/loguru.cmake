
function(build_loguru LOGURU_DIR_VAR DEPENDENCIES_DIR)
    set(${LOGURU_DIR_VAR} ${DEPENDENCIES_DIR}/loguru)
    set(${LOGURU_DIR_VAR} ${DEPENDENCIES_DIR}/loguru PARENT_SCOPE)

    if(NOT EXISTS "${${LOGURU_DIR_VAR}}/loguru.hpp")
        message(STATUS "${${LOGURU_DIR_VAR}}/loguru.hpp was not found")
        message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
    endif()

    file(GLOB_RECURSE LOGURU_SOURCES "${LOGURU_DIR}/loguru.cpp")
    add_library(loguru STATIC ${LOGURU_SOURCES})
    prepare_dependency(NAME "Loguru" TARGETS loguru)
endfunction()