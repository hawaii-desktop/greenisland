find_package(Qt5Gui REQUIRED COMPONENTS Private)
find_package(Qt5Qml REQUIRED COMPONENTS Private)
find_package(Qt5Quick REQUIRED COMPONENTS Private)

# Qt 5.5.x doesn't define Qt5Qml and Qt5Quick private
# include directories, so we have to fix it
foreach(_mod Qml Quick)
    foreach(_dir ${Qt5${_mod}_INCLUDE_DIRS})
        set(_path "${_dir}/Qt${_mod}/${Qt5${_mod}_VERSION_STRING}")
        if(EXISTS ${_path})
            list(APPEND Qt5${_mod}_PRIVATE_INCLUDE_DIRS ${_path})
        endif()

        set(_path "${_dir}/Qt${_mod}/${Qt5${_mod}_VERSION_STRING}/Qt${_mod}")
        if(EXISTS ${_path})
            list(APPEND Qt5${_mod}_PRIVATE_INCLUDE_DIRS ${_path})
        endif()
    endforeach()
endforeach()

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

add_definitions(-Wno-deprecated-declarations)

include(../GreenIslandConfigExtrasLocal.cmake)
include(../GreenIslandMacros.cmake)

set(SOURCES
    screen/fakescreenbackend.cpp
    screen/nativescreenbackend.cpp
    screen/quickscreenmanager.cpp
    screen/screenbackend.cpp
    screen/screenmanager.cpp
    extensions/xdgshell.cpp
    extensions/quickxdgpopupitem.cpp
    extensions/quickxdgsurfaceitem.cpp
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)
    #set(SOURCES ${SOURCES} bufferattacher.cpp)
endif()

greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/wayland.xml
    BASENAME wayland
    PREFIX wl_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland/greenisland.xml
    BASENAME greenisland
    PREFIX greenisland_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland/greenisland-recorder.xml
    BASENAME greenisland-recorder
    PREFIX greenisland_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland/greenisland-screenshooter.xml
    BASENAME greenisland-screenshooter
    PREFIX greenisland_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/xdg-shell.xml
    BASENAME xdg-shell
    PREFIX xdg_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/gtk/gtk-shell.xml
    BASENAME gtk
    PREFIX gtk_
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

add_library(GreenIslandServer SHARED ${SOURCES})
generate_export_header(GreenIslandServer EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/headers/GreenIsland/server/greenislandserver_export.h)
add_library(GreenIsland::Server ALIAS GreenIslandServer)

target_link_libraries(GreenIslandServer
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
        Wayland::Client
        Wayland::Server
        GreenIslandCompositor
    PRIVATE
        Wayland::Cursor
        ${EGL_TARGET}
)

if(EGL_FOUND)
    if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
        target_link_libraries(GreenIslandServer PRIVATE ${OPENGL_LIBRARIES})
    else()
        target_link_libraries(GreenIslandServer PRIVATE ${OPENGLES_LIBRARIES})
    endif()
endif()

if(systemd_FOUND)
    target_link_libraries(GreenIslandServer PRIVATE ${systemd_LIBRARIES})
endif()

target_include_directories(
    GreenIslandServer
        INTERFACE
            "$<INSTALL_INTERFACE:${HAWAII_INCLUDEDIR}>"
            "$<INSTALL_INTERFACE:${GREENISLAND_INCLUDEDIR}>"
)

set_target_properties(GreenIslandServer PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_SOVERSION}
    EXPORT_NAME Server
)

ecm_generate_headers(GreenIslandServer_CamelCase_HEADERS
    HEADER_NAMES
        XdgShell,XdgSurface,XdgPopup
        QuickXdgPopupItem
        QuickXdgSurfaceItem
    PREFIX
        Server
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandServer_HEADERS
    RELATIVE
        extensions
)

ecm_generate_headers(GreenIslandServer_CamelCase_HEADERS
    HEADER_NAMES
        QuickScreenManager
        ScreenBackend,Screen
        ScreenManager
    PREFIX
        Server
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandServer_HEADERS
    RELATIVE
        screen
)

private_headers(GreenIslandServer_PRIVATE_HEADERS
    HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/extensions/quickxdgpopupitem_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/extensions/quickxdgsurfaceitem_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/extensions/xdgshell_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/screen/screenbackend_p.h
        ${CMAKE_CURRENT_SOURCE_DIR}/screen/screenmanager_p.h
        ${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-xdg-shell.h
        ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-shell-server-protocol.h
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland/server
)

install(TARGETS GreenIslandServer
        EXPORT GreenIslandTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${GreenIslandServer_CamelCase_HEADERS}
        DESTINATION ${GREENISLAND_INCLUDEDIR}/Server
        COMPONENT Devel)

install(
    FILES
        ${CMAKE_BINARY_DIR}/headers/GreenIsland/server/greenislandserver_export.h
        ${GreenIslandServer_HEADERS}
    DESTINATION
        ${GREENISLAND_INCLUDEDIR}/server
    COMPONENT
        Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME GreenIslandServer
                      LIB_NAME GreenIslandServer
                      DEPS "core gui quick GreenIslandCompositor"
                      FILENAME_VAR PRI_FILENAME
                      INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}")
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

include(ECMGeneratePkgConfigFile)
ecm_generate_pkgconfig_file(BASE_NAME GreenIslandServer
                            LIB_NAME GreenIslandServer
                            DEPS "Qt5Core Qt5Gui Qt5Quick GreenIslandCompositor"
                            FILENAME_VAR PKGCONFIG_FILENAME
                            INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}"
                            INSTALL)
