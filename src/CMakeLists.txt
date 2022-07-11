add_library(Stdafx STATIC "stdafx.h" "stdafx.cpp")
target_precompile_headers(Stdafx PUBLIC "stdafx.h")
target_link_libraries(Stdafx PUBLIC impl gwca)

add_subdirectory(Utils)
add_subdirectory(Actions)
add_subdirectory(Data)
add_subdirectory(Helper)
add_subdirectory(Main)

file(GLOB DLL_SOURCES
    "*.h"
    "*.cpp")
add_library(HelperBoxDll SHARED ${DLL_SOURCES})
target_compile_definitions(HelperBoxDll PRIVATE "_USRDLL")
target_compile_options(HelperBoxDll PRIVATE /ZI)
target_link_libraries(HelperBoxDll PRIVATE
    Stdafx
    UtilsDll
    TypesDll
    DataDll
    HelperDll
    MainDll
    impl
    gwca
    fmt::fmt
    simpleini
    Urlmon.lib
    Shlwapi.lib
    Wininet.lib
)

if (${ENABLE_WARNINGS})
    target_set_warnings(TARGET HelperBoxDll ENABLE ON AS_ERROR ON)
endif()

if(${ENABLE_LTO})
    target_enable_lto(HelperBoxDll optimized)
endif()
