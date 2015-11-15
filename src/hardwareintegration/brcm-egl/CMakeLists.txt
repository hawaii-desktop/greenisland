include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

add_definitions(-DQT_NO_OPENGL_ES_3)

set(SOURCES
    brcmbuffer.cpp
    brcmeglintegration.cpp
    main.cpp
)

greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/brcm.xml
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
