# System copy of GreenIsland::Platform
if(ENABLE_ONLY_EGLDEVICEINTEGRATION)
    find_package(GreenIsland REQUIRED)
endif()

add_subdirectory(egldeviceintegration)
#if(NOT ENABLE_ONLY_EGLDEVICEINTEGRATION)
#    add_subdirectory(extensions)
#endif()
