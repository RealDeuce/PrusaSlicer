set(_q "")
if(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    set(_q QUIET)
    set(_quietly TRUE)
endif()
find_package(${CMAKE_FIND_PACKAGE_NAME} ${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION} CONFIG ${_q})

if (NOT ${CMAKE_FIND_PACKAGE_NAME}_FOUND)
    # Fall-back solution to find the Cereal serialization library header file
    include(CheckIncludeFileCXX)
    #add_library(cereal INTERFACE)
    target_include_directories(cereal INTERFACE include)

    if (_quietly)
        set(CMAKE_REQUIRED_QUIET ON)
    endif()
    CHECK_INCLUDE_FILE_CXX("cereal/cereal.hpp" HAVE_CEREAL_H)

    if (NOT HAVE_CEREAL_H)
        if (${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
            message(FATAL_ERROR "Cereal library not found. Please install the dependency.")
        elseif(NOT _quietly)
            message(WARNING "Cereal library not found.")
        endif()
    endif ()
endif()
