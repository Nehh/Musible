cmake_minimum_required (VERSION 2.8)

# Our Project Name :)
project (Musible)

# Now we have to get our Vulkan Environment
get_filename_component(VULKAN_SDK "$ENV{VULKAN_SDK}" ABSOLUTE)

# Tell our program where the binarys and headers are
get_filename_component(INC_DIR "${VULKAN_SDK}/include" ABSOLUTE)
get_filename_component(BIN_DIR "${VULKAN_SDK}/bin" ABSOLUTE)

get_filename_component(STB_DIR "$ENV{HOME}/Workspace/stb-master" ABSOLUTE)

# Create a Global Variable in project let vulkan know other file's directory
set(VK_INCLUDE_DIRS ${VK_INCLUDE_DIRS} "${INC_DIR}")

# FIND FONTCONFIG LIBRARY
#if (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIR)
#
#    # in cache already
#    set(FONTCONFIG_FOUND TRUE)
#
#else (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIR)
#
#    if (NOT WIN32)
#        # use pkg-config to get the directories and then use these values
#        # in the find_path() and find_library() calls
#        find_package(PkgConfig)
#        pkg_check_modules(PC_FONTCONFIG fontconfig)
#
#      set(FONTCONFIG_DEFINITIONS ${PC_FONTCONFIG_CFLAGS_OTHER})
#    endif (NOT WIN32)
#
#    find_path(FONTCONFIG_INCLUDE_DIR fontconfig/fontconfig.h
#        PATHS
#        ${PC_FONTCONFIG_INCLUDEDIR}
#        ${PC_FONTCONFIG_INCLUDE_DIRS}
#        /usr/X11/include
#    )
#
#    find_library(FONTCONFIG_LIBRARIES NAMES fontconfig
#        PATHS
#        ${PC_FONTCONFIG_LIBDIR}
#        ${PC_FONTCONFIG_LIBRARY_DIRS}
#    )
#
#    include(FindPackageHandleStandardArgs)
#    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Fontconfig DEFAULT_MSG FONTCONFIG_LIBRARIES FONTCONFIG_INCLUDE_DIR)
#
#    mark_as_advanced(FONTCONFIG_LIBRARIES FONTCONFIG_INCLUDE_DIR)
#
#endif (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIR)
# WE DONT HAVE TO USE IT NOW

# Give a message to Command for checking directorys
message("\n")
message("INC_DIR = ${INC_DIR}")
message("STB_DIR = ${STB_DIR}")
message("\n")

# We have to include which Vulkan wants to include
include_directories (
    ${XCB_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${INC_DIR}
    ${STB_DIR}
)

# We have to make program faster on Release mode or strechable on Debug mode
set(CMAKE_BUILD_TYPE "Debug")

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g -Wall -Wno-unused-variable -pthread")
else( CMAKE_BUILD_TYPE STREQUAL "Debug" )
        set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O2 -pthread -fopenmp")
endif( CMAKE_BUILD_TYPE STREQUAL "Debug" )
message("Build Type:${CMAKE_BUILD_TYPE} ${CMAKE_C_FLAGS}")

# Take care of diffrent lib between mode Release and Debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    get_filename_component(LIB_DIR "${VULKAN_SDK}/../source/lib" ABSOLUTE)
else()
    get_filename_component(LIB_DIR "${VULKAN_SDK}/lib" ABSOLUTE)
endif()
message("LIB_DIR = ${LIB_DIR}")
link_directories(${LIB_DIR})

# Maybe sometime we add a Windows Support :)
set (LIBRARIES "vulkan")

# We won't want GLFW create their documents、tests and examples in our game :)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Now we get the GLFW package from auto-search
find_package(glfw3 3.2 REQUIRED)
find_package(PNG 1.6.23 REQUIRED)

# Linker's about ready, now it's time founding our source code
file(GLOB_RECURSE Musible_SRC
    "./Source/*.c"
    "./Source/*.h"
)


# Now we can build our guys :D
add_executable(Musible ${Musible_SRC})

# Set our output folder to someplace tidy
set_target_properties( Musible
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../Musible/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../Musible/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../Musible"
)

# And find their partners :D
target_link_libraries(Musible ${PNG_LIBRARY})
target_link_libraries(Musible glfw)
target_link_libraries(Musible ${LIBRARIES})
#target_link_libraries(Musible ${FONTCONFIG_LIBRARIES})
# we dont have to use this now

# message("Librarie Directories:${PNG_LIBRARY}")

# Copy Static Resources to Binary Directory
file(COPY "${CMAKE_BINARY_DIR}/../Static/" DESTINATION "${CMAKE_BINARY_DIR}/../Musible")
