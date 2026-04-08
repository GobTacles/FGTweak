# This file configures the "build" of this SKSE plugin
# - how to compile the C++ plugin (in addition to settings found in CMakePresets.json)
# - where to output the resulting .dll file

# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.28)
# 3.28 for c++23 and CMAKE_CXX_SCAN_FOR_MODULES

# [BEGIN set_project_name]
# Set your project name. This will be the name of your SKSE .dll file.
project(FGTweak VERSION 0.1.0 LANGUAGES CXX)
# [END set_project_name]

# add source files to this set manually for best dependency checks etc 
set(SOURCE_MAIN_CPP
    src/fg_main.cpp
    src/fg_logutil.cpp
)
# or use file(GLOB_RECURSE SOURCE_MAIN_CPP src/*.cpp) for lazy

# #
# YOU DO NOT NEED TO EDIT ANYTHING BELOW HERE
# #

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Setup your SKSE plugin as an SKSE plugin!
find_package(CommonLibSSE CONFIG REQUIRED)

# see build\release\vcpkg_installed\x64-windows-static-md\share\CommonLibSSE\CommonLibSSE.cmake
add_commonlibsse_plugin(${PROJECT_NAME})

target_sources("${PROJECT_NAME}"        PRIVATE ${SOURCE_MAIN_CPP} )


add_library(my_compiler_flags INTERFACE)
target_compile_features(my_compiler_flags INTERFACE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(my_compiler_flags INTERFACE include/PCH.h) # precompiled headers
# Include the header files from the include/ directory
target_include_directories(my_compiler_flags INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Disable warning C4100 unreferenced formal parameter
if (MSVC)
    target_compile_options(my_compiler_flags INTERFACE /wd4100)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(my_compiler_flags INTERFACE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING) # silence spam warning STL4043 for debug compile in logger
    endif()
endif()

target_link_libraries(${PROJECT_NAME}         PUBLIC my_compiler_flags)

# +++++ output folders 

# If you're not using a mod manager, you probably want the SKSE plugin to go
# inside of your Skyrim "Data" folder.
#
# To do this automatically, set the `SKYRIM_FOLDER` environment variable
# to the path of your Skyrim Special Edition folder
if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}\\Data")
else()
    message(STATUS "SKYRIM_FOLDER environment variable is not defined or Data is not a subdirectory")
endif()

# If you're using Mod Organizer 2 or Vortex, you might want this to go inside
# of your "mods" folder, inside of a subfolder named "<your mod>".
#
# To do this automatically, set the `SKYRIM_MODS_FOLDER` environment variable
# to the path of your "mods" folder. Reboot your PC after changing environment variables
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}\\${PROJECT_NAME}")
    message(STATUS "ENV{SKYRIM_MODS_FOLDER} -> OUTPUT_FOLDER=${OUTPUT_FOLDER}")
else()
    message(STATUS "SKYRIM_MODS_FOLDER environment variable is not defined or is not a directory")
endif()

# Otherwise, you can set OUTPUT_FOLDER to any place you'd like :)
# set(OUTPUT_FOLDER "C:/path/to/any/folder")

# When your SKSE .dll is compiled, this will automatically copy the .dll into your mods folder.
# Only works if you configure DEPLOY_ROOT above (or set the SKYRIM_MODS_FOLDER environment variable)
if(NOT DEFINED OUTPUT_FOLDER)
    message(STATUS "OUTPUT_FOLDER is not defined")
endif()
if(DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DESTINATION_FOLDER "${OUTPUT_FOLDER}\\SKSE\\Plugins")

    message(STATUS "SKSE plugin output destination folder: ${DESTINATION_FOLDER}")

    # copy ${PROJECT_NAME}.dll
    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DESTINATION_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DESTINATION_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMENT "Copying DLL to destination folder ${DESTINATION_FOLDER}"
        VERBATIM
    )

    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DESTINATION_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()
