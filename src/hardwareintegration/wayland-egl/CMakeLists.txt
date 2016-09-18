include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/.."
    "${CMAKE_CURRENT_BINARY_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers"
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers/GreenIsland"
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
