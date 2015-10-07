include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/src/platform
)

add_definitions(-DMESA_EGL_NO_X11_HEADERS)

set(SOURCES
    eglfsmaliintegration.cpp
    main.cpp
)

add_library(mali SHARED MODULE ${SOURCES})
target_link_libraries(mali
    GreenIsland::Platform
    EGL::EGL
    ${OPENGLES_LIBRARIES}
)

install(TARGETS mali
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
