cmake_minimum_required(VERSION 3.21)

project("HelperBox" VERSION 0.1.0 LANGUAGES C CXX)

# Global CMake variables are set here
set(CMAKE_CXX_STANDARD              17)
set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_CXX_EXTENSIONS            OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)

set(HELPERBOXDLL_VERSION ${CMAKE_PROJECT_VERSION})

add_compile_definitions(
    "NOMINMAX"
    "_USE_MATH_DEFINES"
    "WIN32_LEAN_AND_MEAN"
    "_CRT_SECURE_NO_WARNINGS"
    "HELPERBOXDLL_VERSION=\"${HELPERBOXDLL_VERSION}\"")

# Options
option(ENABLE_WARNINGS "Enable to add warnings to a target." ON)

### CMAKE MODULES
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
include(FetchContent)
include(ConfigSafeGuards)
include(Warnings)

### EXTERNAL LIBS
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.10.5
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 8.1.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fmt)

# Outputs dll, exe, and pdb into a /bin/config folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(app)

target_compile_definitions(HelperBoxDll PRIVATE "$<$<CONFIG:DEBUG>:HELPERBOX_DEBUG>")
