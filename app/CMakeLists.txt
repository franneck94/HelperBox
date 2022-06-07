

FILE(GLOB SOURCES
    "*.h"
    "*.cpp")

add_executable(HelperBox)
target_sources(HelperBox PRIVATE ${SOURCES})
target_precompile_headers(HelperBox PRIVATE "stdafx.h")

set_target_properties(HelperBox PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(HelperBox PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
set_target_properties(HelperBox PROPERTIES LINK_FLAGS_MINRELSIZE "/SUBSYSTEM:WINDOWS")

target_link_libraries(HelperBox PRIVATE
    HelperBoxDll
    nlohmann_json::nlohmann_json
    # libs:
    Ws2_32.lib
    Shlwapi.lib
    kernel32.lib
    user32.lib
    gdi32.lib
    winspool.lib
    comdlg32.lib
    advapi32.lib
    shell32.lib
    ole32.lib
    oleaut32.lib
    uuid.lib
    odbc32.lib
    odbccp32.lib)

if (${ENABLE_WARNINGS})
    target_set_warnings(TARGET HelperBox ENABLE ON AS_ERROR OFF)
endif()

if(${ENABLE_LTO})
    target_enable_lto(HelperBox optimized)
endif()

# Copy <filename> to build directory
set(copy_source_dir "${PROJECT_SOURCE_DIR}/")
set(copy_dest_dir_rel "${PROJECT_SOURCE_DIR}/bin/Release")
set(copy_dest_dir_debug "${PROJECT_SOURCE_DIR}/bin/Debug")
set(copy_file_name "select_instance.py")
# release copy
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${copy_dest_dir_rel}
)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${copy_source_dir}/${copy_file_name}" "${copy_dest_dir_rel}/${copy_file_name}"
    COMMENT "Copying ${copy_file_name} to build directory"
)
# debug copy
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${copy_dest_dir_debug}
)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${copy_source_dir}/${copy_file_name}" "${copy_dest_dir_debug}/${copy_file_name}"
    COMMENT "Copying ${copy_file_name} to build directory"
)
