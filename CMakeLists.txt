cmake_minimum_required(VERSION 3.19)

# ---- Cache build vars ----

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/ports" CACHE STRING "")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/triplets" CACHE STRING "")
set(VCPKG_TARGET_TRIPLET "x64-windows-static-md-custom" CACHE STRING "")

# ---- Project ----

project(
	EngineFixes
	VERSION 5.4.1
	LANGUAGES CXX
)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/Version.h"
	@ONLY
)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
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
	TOML_WINDOWS_COMPAT=0
)

if (MSVC)
	add_compile_definitions(
		_UNICODE
	)

	string(REPLACE " " ";" MY_CXX_FLAGS "${VCPKG_CXX_FLAGS}")
	string(REPLACE " " ";" MY_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG}")
	string(REPLACE " " ";" MY_CXX_FLAGS_RELEASE "${VCPKG_CXX_FLAGS_RELEASE}")

	add_compile_options(
		/MP	# Build with Multiple Processes
		/sdl	# Enable Additional Security Checks
		/Zi	# Debug Information Format

		/permissive-	# Standards conformance

		/Zc:alignedNew	# C++17 over-aligned allocation
		/Zc:auto	# Deduce Variable Type
		/Zc:char8_t
		/Zc:__cplusplus	# Enable updated __cplusplus macro
		/Zc:externC
		/Zc:externConstexpr	# Enable extern constexpr variables
		/Zc:forScope	# Force Conformance in for Loop Scope
		/Zc:hiddenFriend
		/Zc:implicitNoexcept	# Implicit Exception Specifiers
		/Zc:lambda
		/Zc:noexceptTypes	# C++17 noexcept rules
		/Zc:preprocessor	# Enable preprocessor conformance mode
		/Zc:referenceBinding	# Enforce reference binding rules
		/Zc:rvalueCast	# Enforce type conversion rules
		/Zc:sizedDealloc	# Enable Global Sized Deallocation Functions
		/Zc:strictStrings	# Disable string literal type conversion
		/Zc:ternary	# Enforce conditional operator rules
		/Zc:threadSafeInit	# Thread-safe Local Static Initialization
		/Zc:tlsGuards
		/Zc:trigraphs	# Trigraphs Substitution
		/Zc:wchar_t	# wchar_t Is Native Type

		/experimental:external
		/external:anglebrackets
		/external:W0

		/W4	# Warning level
		/WX	# Warning level (warnings are errors)

		${MY_CXX_FLAGS}
		"$<$<CONFIG:DEBUG>:${MY_CXX_FLAGS_DEBUG}>"
		"$<$<CONFIG:RELEASE>:${MY_CXX_FLAGS_RELEASE};/Zc:inline;/JMC->"
	)

	# fix a warning with /Ob2 being overridden with /Ob3
	string(REPLACE "/Ob2" "/Ob3" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

	# fix a warning with /EHsc being overridden with /EHa
	string(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# ---- Dependencies ----

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_DEBUG_RUNTIME ON)

add_subdirectory("$ENV{CommonLibSSEPath}" CommonLibSSE)

find_package(AutoTOML REQUIRED CONFIG)
find_package(Boost
	REQUIRED
	COMPONENTS
		regex
)
find_package(TBB
	REQUIRED
	COMPONENTS
		tbb
		tbbmalloc
	CONFIG
)
find_package(xbyak REQUIRED CONFIG)

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
	${PROJECT_NAME}.toml
	.clang-format
)

target_compile_features(
	${PROJECT_NAME}
	PRIVATE
		cxx_std_20
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
		AutoTOML::AutoTOML
		Boost::regex
		CommonLibSSE::CommonLibSSE
		TBB::tbb
		TBB::tbbmalloc
		xbyak::xbyak
)

if (MSVC)
	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		src/PCH.h
)

# ---- File copying ----

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> $ENV{Skyrim64Path}/Data/SKSE/Plugins/
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> $ENV{Skyrim64Path}/Data/SKSE/Plugins/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.toml $ENV{Skyrim64Path}/Data/SKSE/Plugins/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}_preload.txt $ENV{Skyrim64Path}/Data/SKSE/Plugins/
)
