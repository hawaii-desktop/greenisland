include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerConfigExtrasLocal.cmake)
include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerMacros.cmake)

# Avoid warnings regarding wl_buffer being deprecated
add_definitions(-Wno-deprecated-declarations)

set(SOURCES
    buffer.cpp
    clientconnection.cpp
    compositor.cpp
    cursortheme.cpp
    keyboard.cpp
    fullscreenshell.cpp
    output.cpp
    pointer.cpp
    region.cpp
    registry.cpp
    screencaster.cpp
    screenshooter.cpp
    seat.cpp
    shm.cpp
    shmformats_p.cpp
    shmpool.cpp
    surface.cpp
    touch.cpp
)

greenisland_add_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/wayland.xml
    BASENAME wayland
    PREFIX wl_
)
greenisland_add_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/fullscreen-shell.xml
    BASENAME fullscreen-shell
    PREFIX _wl_
)
greenisland_add_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland/greenisland-screencaster.xml
    BASENAME greenisland-screencaster
    PREFIX greenisland_
)
greenisland_add_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland/greenisland-screenshooter.xml
    BASENAME greenisland-screenshooter
    PREFIX greenisland_
)

add_library(GreenIslandClient SHARED ${SOURCES})
generate_export_header(GreenIslandClient EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/headers/GreenIsland/client/greenislandclient_export.h)
add_library(GreenIsland::Client ALIAS GreenIslandClient)

target_link_libraries(GreenIslandClient
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Wayland::Client
        Wayland::Cursor
)

target_include_directories(
    GreenIslandClient
        INTERFACE
            "$<INSTALL_INTERFACE:${HAWAII_INCLUDEDIR}>"
            "$<INSTALL_INTERFACE:${GREENISLAND_INCLUDEDIR}>"
)

set_target_properties(GreenIslandClient PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_SOVERSION}
    EXPORT_NAME Client
)

ecm_generate_headers(GreenIslandClient_CamelCase_HEADERS
    HEADER_NAMES
        Buffer
        ClientConnection
        Compositor
        CursorTheme
        FullScreenShell
        Keyboard
        Output
        Pointer
        Region
        Registry
        Screencaster,Screencast
        Screenshooter,Screenshot
        Seat
        Shm
        ShmPool
        Surface
        Touch
    PREFIX
        Client
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandClient_HEADERS
)

private_headers(GreenIslandClient_PRIVATE_HEADERS
    HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/buffer_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/clientconnection_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/cursortheme_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/keyboard_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/fullscreenshell_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/output_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/pointer_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/region_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/registry_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/screencaster_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/screenshooter_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/seat_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shm_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shmformats_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shmpool_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/surface_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/touch_p.h
        ${CMAKE_CURRENT_BINARY_DIR}/qwayland-wayland.h
        ${CMAKE_CURRENT_BINARY_DIR}/qwayland-fullscreen-shell.h
        ${CMAKE_CURRENT_BINARY_DIR}/qwayland-greenisland-screencaster.h
        ${CMAKE_CURRENT_BINARY_DIR}/qwayland-greenisland-screenshooter.h
        ${CMAKE_CURRENT_BINARY_DIR}/wayland-wayland-client-protocol.h
        ${CMAKE_CURRENT_BINARY_DIR}/wayland-fullscreen-shell-client-protocol.h
        ${CMAKE_CURRENT_BINARY_DIR}/wayland-greenisland-screencaster-client-protocol.h
        ${CMAKE_CURRENT_BINARY_DIR}/wayland-greenisland-screenshooter-client-protocol.h
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland/client
)


install(TARGETS GreenIslandClient
        EXPORT GreenIslandClientTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${GreenIslandClient_CamelCase_HEADERS}
        DESTINATION ${GREENISLAND_INCLUDEDIR}/Client
        COMPONENT Devel)

install(
    FILES
        ${CMAKE_BINARY_DIR}/headers/GreenIsland/client/greenislandclient_export.h
        ${GreenIslandClient_HEADERS}
    DESTINATION
        ${GREENISLAND_INCLUDEDIR}/client
    COMPONENT
        Devel
)

# CMake package config
set(GreenIslandClient_CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/GreenIslandClient")
ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/GreenIslandClientConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandClientConfig.cmake"
                                  INSTALL_DESTINATION ${GreenIslandClient_CMAKECONFIG_INSTALL_DIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandClientConfig.cmake"
        DESTINATION "${GreenIslandClient_CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)
install(EXPORT GreenIslandClientTargets
        DESTINATION "${GreenIslandClient_CMAKECONFIG_INSTALL_DIR}"
        FILE GreenIslandClientTargets.cmake NAMESPACE GreenIsland::)

# qmake pri file
include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME GreenIslandClient
                      LIB_NAME GreenIslandClient
                      DEPS "core gui"
                      FILENAME_VAR PRI_FILENAME
                      INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}")
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

# pkg-config file
include(ECMGeneratePkgConfigFile)
#ecm_generate_pkgconfig_file(BASE_NAME GreenIslandClient
#                            LIB_NAME GreenIslandClient
#                            DEPS "Qt5Core Qt5Gui"
#                            FILENAME_VAR PKGCONFIG_FILENAME
#                            INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}"
#                            INSTALL)
