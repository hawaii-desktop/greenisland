include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
    ${WAYLAND_SERVER_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(-DQT_PLUGIN)

set(SOURCES
    main.cpp
    qopenglwindow.cpp
    textureblitter.cpp
    compositor.cpp
    integration.cpp
)

wayland_add_protocol_server(SOURCES
    ${CMAKE_SOURCE_DIR}/protocol/desktop-shell.xml
    desktop-shell
)
wayland_add_protocol_client(SOURCES
    ${CMAKE_SOURCE_DIR}/protocol/desktop-shell.xml
    desktop-shell
)

qt5_add_resources(RESOURCES classic.qrc)

add_library(classic SHARED ${SOURCES} ${RESOURCES})
qt5_use_modules(classic Compositor)
set_target_properties(classic PROPERTIES
    PREFIX ""
    LINK_FLAGS -L${WAYLAND_SERVER_LIBDIR}
)
target_link_libraries(classic
    ${WAYLAND_SERVER_LIBRARIES}
    GreenIsland
)

install(TARGETS classic
    DESTINATION ${CMAKE_INSTALL_PLUGINSDIR}/greenisland/compositors)
