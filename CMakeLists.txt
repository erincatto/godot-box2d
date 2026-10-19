cmake_minimum_required(VERSION 3.24)

project(godot-box2d LANGUAGES C CXX)

include(FetchContent)

# The source overrides virtuals whose signatures only exist from 4.7 on
set(GODOTCPP_DEFAULT_API_VERSION "4.7")

# Box2D is linked into the extension, never shipped beside it
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

FetchContent_Declare(
	godot-cpp
	GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
	GIT_TAG 10.0.0-stable
	GIT_SHALLOW TRUE
)

FetchContent_Declare(
	box2d
	GIT_REPOSITORY https://github.com/erincatto/box2d.git
	GIT_TAG v3.2.0
	GIT_SHALLOW TRUE
)

# godot-cpp first, it caches the MSVC runtime that Box2D has to match
FetchContent_MakeAvailable(godot-cpp box2d)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(platform windows)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(platform linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(platform macos)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
	set(platform ios)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
	set(platform android)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
	set(platform web)
else()
	message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(EMSCRIPTEN)
	# The override godot-cpp applies to its own project. Emscripten refuses shared libraries without it.
	set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
	set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-sSIDE_MODULE=1")
	set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-sSIDE_MODULE=1")
	set(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")
endif()

file(GLOB sources CONFIGURE_DEPENDS
	src/*.cpp
	src/bodies/*.cpp
	src/joints/*.cpp
	src/shapes/*.cpp
	src/spaces/*.cpp
)

add_library(godot-box2d SHARED ${sources})
target_include_directories(godot-box2d PRIVATE src)
target_link_libraries(godot-box2d PRIVATE godot-cpp box2d)

file(GLOB doc_xml CONFIGURE_DEPENDS doc_classes/*.xml)
if(doc_xml AND GODOTCPP_TARGET MATCHES "editor|template_debug")
	target_doc_sources(godot-box2d "${doc_xml}")
endif()

# Must match the names in demo/bin/godot-box2d.gdextension, which nothing regenerates.
# That is godot-cpp's suffix without .dev, and no arch on a universal macOS build.
get_target_property(arch godot-cpp GODOTCPP_ARCH)
set(suffix ".${platform}.${GODOTCPP_TARGET}")
if(GODOTCPP_PRECISION STREQUAL "double")
	string(APPEND suffix ".double")
endif()
if(NOT arch STREQUAL "universal")
	string(APPEND suffix ".${arch}")
endif()
if(NOT GODOTCPP_THREADS)
	string(APPEND suffix ".nothreads")
endif()

if(WIN32)
	set(prefix "")
else()
	set(prefix "lib")
endif()

# The generator expression stops multi config generators from appending a config folder
set(output_dir "$<1:${CMAKE_CURRENT_SOURCE_DIR}/demo/bin/${platform}>")

set_target_properties(godot-box2d PROPERTIES
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
	CXX_VISIBILITY_PRESET ${GODOTCPP_SYMBOL_VISIBILITY}
	PREFIX "${prefix}"
	OUTPUT_NAME "godot-box2d${suffix}"
	LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
	RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
)
