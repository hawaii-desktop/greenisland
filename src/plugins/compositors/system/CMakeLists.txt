include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${WAYLAND_SERVER_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(
    -DQT_PLUGIN
    -DQT_COMPOSITOR_QUICK
)

set(SOURCES
    main.cpp
    systemcompositor.cpp
    systemcompositorserver.cpp
    systemclient.cpp
)

wayland_add_protocol_server(
    SOURCES
    ${CMAKE_SOURCE_DIR}/protocol/system-compositor.xml
    system-compositor
)
wayland_add_protocol_client(
    SOURCES
    ${CMAKE_SOURCE_DIR}/protocol/system-compositor.xml
    system-compositor
)

wayland_add_protocol_server(
    SOURCES
    ${CMAKE_SOURCE_DIR}/protocol/display-manager.xml
    display-manager
)
wayland_add_protocol_client(
    SOURCES
    ${CMAKE_SOURCE_DIR}/protocol/display-manager.xml
    display-manager
)

qt5_add_resources(RESOURCES system.qrc)

add_library(system SHARED ${SOURCES} ${RESOURCES})
qt5_use_modules(system Quick Compositor)
set_target_properties(system PROPERTIES
    PREFIX ""
    LINK_FLAGS -L${WAYLAND_SERVER_LIBDIR}
)
target_link_libraries(system
    ${VibeCore_LIBRARIES}
    ${WAYLAND_SERVER_LIBRARIES}
    GreenIsland
)

install(TARGETS system
    DESTINATION ${CMAKE_INSTALL_PLUGINSDIR}/greenisland/compositors)
