include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerConfigExtrasLocal.cmake)
include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerMacros.cmake)

set(SOURCES
    clientconnection.cpp
    cursortheme.cpp
    pointer.cpp
    registry.cpp
    seat.cpp
    shmpool.cpp
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

add_library(GreenIslandClient SHARED ${SOURCES})
generate_export_header(GreenIslandClient EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/headers/GreenIsland/client/greenislandclient_export.h)
add_library(GreenIsland::Client ALIAS GreenIslandClient)

target_link_libraries(GreenIslandClient
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
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
        ClientConnection
        CursorTheme
        FullScreenShell
        Pointer
        Registry
        Seat
        ShmPool
    PREFIX
        Client
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandClient_HEADERS
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
ecm_generate_pkgconfig_file(BASE_NAME GreenIslandClient
                            LIB_NAME GreenIslandClient
                            DEPS "Qt5Core Qt5Gui"
                            FILENAME_VAR PKGCONFIG_FILENAME
                            INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}"
                            INSTALL)
