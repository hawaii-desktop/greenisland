include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerConfigExtrasLocal.cmake)
include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerMacros.cmake)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
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

add_library(plasma MODULE ${SOURCES})
target_link_libraries(plasma
                      GreenIsland::Server
                      Wayland::Server)

install(TARGETS plasma
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/extensions)
