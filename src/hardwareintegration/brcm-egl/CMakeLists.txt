include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/.."
    "${CMAKE_CURRENT_BINARY_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/../../headers"
    "${CMAKE_CURRENT_BINARY_DIR}/../../headers/GreenIsland"
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

add_definitions(-DQT_NO_OPENGL_ES_3)

set(SOURCES
    brcmbuffer.cpp
    brcmeglintegration.cpp
    main.cpp
)

greenisland_add_server_protocol(SOURCES
    PROTOCOL "${CMAKE_CURRENT_SOURCE_DIR}/data/protocols/qt/brcm.xml"
    BASENAME brcm
    PREFIX qt_
)

add_library(brcm-egl MODULE ${SOURCES})
target_link_libraries(brcm-egl
                      GreenIsland::Compositor
                      Wayland::Server
		      Wayland::Egl
                      BcmHost::BcmHost
                      BcmHost::EGL
                      BcmHost::GLESv2)

install(TARGETS brcm-egl
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/hardwareintegration)
