include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    main.cpp
    waylandeglclientbufferintegration.cpp
)

add_library(wayland-egl MODULE ${SOURCES})
target_link_libraries(wayland-egl
                      GreenIsland::Compositor
                      GreenIsland::Platform
                      Wayland::Server
		      Wayland::Egl
                      EGL::EGL)

install(TARGETS wayland-egl
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/hardwareintegration)
