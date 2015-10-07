include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/src/platform
)

add_definitions(-DMESA_EGL_NO_X11_HEADERS)
add_definitions(-DLINUX=1 -DEGL_API_FB=1)

set(SOURCES
    eglfsvivintegration.cpp
    main.cpp
)

add_library(viv SHARED MODULE ${SOURCES})
target_link_libraries(viv
    GreenIsland::Platform
    EGL::EGL
    ${OPENGLES_LIBRARIES}
    GAL
)

install(TARGETS viv
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
