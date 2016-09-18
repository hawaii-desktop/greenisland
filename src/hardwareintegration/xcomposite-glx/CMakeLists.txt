include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/.."
    "${CMAKE_CURRENT_BINARY_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers"
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers/GreenIsland"
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${X11_X11_INCLUDE_PATH}
    ${X11_Xcomposite_INCLUDE_PATH}
)

set(SOURCES
    main.cpp
    xcompositeglxintegration.cpp
)

add_library(xcomposite-glx MODULE ${SOURCES})
target_link_libraries(xcomposite-glx
                      GreenIsland::Compositor
                      GreenIsland::XComposite
                      Wayland::Server
                      ${X11_X11_LIB}
                      ${X11_Xcomposite_LIB}
                      ${OPENGL_LIBRARIES})

install(TARGETS xcomposite-glx
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/hardwareintegration)
