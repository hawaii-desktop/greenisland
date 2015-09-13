include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandConfigExtrasLocal.cmake)
include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandMacros.cmake)

include_directories(
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/screen
    ${CMAKE_BINARY_DIR}/src/server
    ${CMAKE_BINARY_DIR}/src/server/greenisland
)

set(SOURCES
    plugin.cpp
    plasmaeffects.cpp
    plasmashell.cpp
    plasmasurface.cpp
)

greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/plasma/plasma-shell.xml
    BASENAME plasma-shell
    PREFIX org_kde_plasma_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/plasma/plasma-effects.xml
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
