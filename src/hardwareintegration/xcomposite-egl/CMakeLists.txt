include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${X11_X11_INCLUDE_PATH}
    ${X11_Xcomposite_INCLUDE_PATH}
)

set(SOURCES
    main.cpp
    xcompositeeglintegration.cpp
)

add_library(xcomposite-egl MODULE ${SOURCES})
target_link_libraries(xcomposite-egl
                      GreenIsland::Compositor
                      GreenIsland::XComposite
                      Wayland::Server
                      ${X11_X11_LIB}
                      ${X11_Xcomposite_LIB}
                      EGL::EGL)

install(TARGETS xcomposite-egl
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/hardwareintegration)
