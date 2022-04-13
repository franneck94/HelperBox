add_subdirectory(Utils)
add_subdirectory(Helper)
add_subdirectory(Main)

file(GLOB DLL_SOURCES
    "*.h"
    "*.cpp")
add_library(HelperBoxDll SHARED ${DLL_SOURCES})
target_compile_definitions(HelperBoxDll PUBLIC "_USRDLL")
target_compile_options(HelperBoxDll PUBLIC /ZI)
target_link_libraries(HelperBoxDll PUBLIC
    UtilsDll
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
    target_set_warnings(TARGET HelperBoxDll ENABLE ON AS_ERROR OFF)
endif()

if(${ENABLE_LTO})
    target_enable_lto(HelperBoxDll optimized)
endif()
