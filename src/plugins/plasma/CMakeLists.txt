include_directories(
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/libgreenisland
    ${CMAKE_SOURCE_DIR}/src/libgreenisland/screen
    ${CMAKE_BINARY_DIR}/src/libgreenisland
    ${CMAKE_BINARY_DIR}/src/libgreenisland/greenisland
)

set(SOURCES
    plugin.cpp
    plasmaeffects.cpp
    plasmashell.cpp
    plasmasurface.cpp
)

ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/plasma-shell.xml
    BASENAME plasma-shell
    PREFIX org_kde_plasma_
)
ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/plasma-effects.xml
    BASENAME plasma-effects
    PREFIX org_kde_plasma_
)

add_library(plasma SHARED ${SOURCES})
set_target_properties(plasma PROPERTIES PREFIX "")
target_link_libraries(plasma
                      Qt5::Compositor
                      GreenIsland::GreenIsland
                      Wayland::Server)

install(TARGETS plasma
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland)
