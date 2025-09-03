cmake_minimum_required(VERSION 3.20)
set(NAME "EngineFixes" CACHE STRING "")
set(VERSION 7.0.17 CACHE STRING "")

# ---- Options ----
option(COPY_BUILD "Copy the build output to the Skyrim directory." TRUE)
option(BUILD_SKYRIMAE "Build for Skyrim AE" OFF)

# ---- Cache build vars ----
macro(set_from_environment VARIABLE)
    if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
        set(${VARIABLE} $ENV{${VARIABLE}})
    endif()
endmacro()

macro(find_commonlib_path)
    if(CommonLibName AND NOT ${CommonLibName} STREQUAL "")
        # Check extern
        find_path(CommonLibPath
            include/REL/Relocation.h
            PATHS external/${CommonLibName}
            extern/${CommonLibName})

        if(${CommonLibPath} STREQUAL "CommonLibPath-NOTFOUND")
            # Check path
            set_from_environment(${CommonLibName}Path)
            set(CommonLibPath ${${CommonLibName}Path})
        endif()
    endif()
endmacro()

set_from_environment(VCPKG_ROOT)

if(BUILD_SKYRIMAE)
    add_compile_definitions(SKYRIM_AE)
    add_compile_definitions(SKYRIM_SUPPORT_AE)
    set(SkyrimVersion "Skyrim AE")
else()
    set(SkyrimVersion "Skyrim SSE")
endif()

set(CommonLibName "CommonLibSSE")
set_from_environment(Skyrim64Path)
set(SkyrimPath ${Skyrim64Path})

set(VERSION ${VERSION})

find_commonlib_path()
message(
    STATUS
    "Building ${NAME} ${VERSION} for ${SkyrimVersion} at ${SkyrimPath} with ${CommonLibName} at ${CommonLibPath}."
)

if(DEFINED VCPKG_ROOT)
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
else()
    message(
        WARNING
        "Variable VCPKG_ROOT is not set. Continuing without vcpkg."
    )
endif()

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# ---- Project ----
project(
    ${NAME}
    VERSION ${VERSION}
    LANGUAGES CXX
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
        "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# ---- Globals ----
add_compile_definitions(
    SKSE_SUPPORT_XBYAK
    REX_OPTION_INI
    REX_OPTION_TOML
)

if(MSVC)
    if(NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
        add_compile_options(
            /MP # Build with Multiple Processes
        )
    endif()
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)

# ---- Dependencies ----
if(DEFINED CommonLibPath AND NOT ${CommonLibPath} STREQUAL "" AND IS_DIRECTORY ${CommonLibPath})
    add_subdirectory(${CommonLibPath} ${CommonLibName})
else()
    message(
        FATAL_ERROR
        "Variable ${CommonLibName}Path is not set."
    )
endif()


find_package(TBB CONFIG REQUIRED)
find_package(safetyhook CONFIG REQUIRED)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost CONFIG REQUIRED COMPONENTS regex)

# ---- Add source files ----
include(cmake/headerlist.cmake)
include(cmake/sourcelist.cmake)

source_group(
    TREE
    ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
    ${headers}
    ${sources}
)

source_group(
    TREE
    ${CMAKE_CURRENT_BINARY_DIR}
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# ---- Create DLL ----
add_library(
    ${PROJECT_NAME}
    SHARED
    ${headers}
    ${sources}
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    .clang-format
    .editorconfig
)

target_compile_features(
    ${PROJECT_NAME}
    PRIVATE
    cxx_std_23
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    _UNICODE
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    ${CommonLibName}::${CommonLibName}
    safetyhook::safetyhook
    TBB::tbb
    TBB::tbbmalloc
    Boost::regex
)

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE
    src/PCH.h
)

if(MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        /sdl # Enable Additional Security Checks
        /utf-8 # Set Source and Executable character sets to UTF-8
        /Zi # Debug Information Format

        /permissive- # Standards conformance
        /Zc:preprocessor # Enable preprocessor conformance mode

        /wd4200 # nonstandard extension used : zero-sized array in struct/union

        "$<$<CONFIG:DEBUG>:>"
        "$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
    )

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
        "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
        "$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
endif()

# ---- Post build ----
if(COPY_BUILD)
    if(DEFINED SkyrimPath)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${SkyrimPath}/${NAME}/SKSE/Plugins/
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> ${SkyrimPath}/${NAME}/SKSE/Plugins/
        )
    else()
        message(
            WARNING
            "Variable ${SkyrimPath} is not defined. Skipping post-build copy command."
        )
    endif()
endif()