include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

add_definitions(-Wno-deprecated-declarations)

include(../GreenIslandConfigExtrasLocal.cmake)
include(../GreenIslandMacros.cmake)

set(SOURCES
    abstractplugin.cpp
    applicationmanager.cpp
    clientwindow.cpp
    compositor.cpp
    compositor_p.cpp
    compositorsettings.cpp
    diagnostic_p.cpp
    homeapplication.cpp
    keybinding.cpp
    keybindings.cpp
    logging.cpp
    output.cpp
    outputwindow.cpp
    windowview.cpp
    shellwindow.cpp
    utilities.cpp
    client/wlclientconnection.cpp
    client/wlcursortheme.cpp
    client/wlpointer.cpp
    client/wlregistry.cpp
    client/wlseat.cpp
    client/wlshmpool.cpp
    screen/screenbackend.cpp
    screen/screenconfiguration.cpp
    screen/fakescreenbackend.cpp
    screen/nativescreenbackend.cpp
    screen/screenmanager.cpp
    protocols/fullscreen-shell/fullscreenshellclient.cpp
    protocols/greenisland/greenislandapps.cpp
    protocols/greenisland/greenislandwindow.cpp
    protocols/greenisland/greenislandwindows.cpp
    protocols/greenisland/greenislandrecorder.cpp
    protocols/greenisland/greenislandscreenshooter.cpp
    protocols/wl-shell/wlshell.cpp
    protocols/wl-shell/wlshellsurface.cpp
    protocols/wl-shell/wlshellsurfacegrabber.cpp
    protocols/wl-shell/wlshellsurfacepopupgrabber.cpp
    protocols/wl-shell/wlshellsurfacemovegrabber.cpp
    protocols/wl-shell/wlshellsurfaceresizegrabber.cpp
    protocols/wayland/wlsubcompositor.cpp
    protocols/wayland/wlsubsurface.cpp
    protocols/xdg-shell/xdgshell.cpp
    protocols/xdg-shell/xdgsurface.cpp
    protocols/xdg-shell/xdgsurfacegrabber.cpp
    protocols/xdg-shell/xdgsurfacemovegrabber.cpp
    protocols/xdg-shell/xdgsurfaceresizegrabber.cpp
    protocols/xdg-shell/xdgpopup.cpp
    protocols/xdg-shell/xdgpopupgrabber.cpp
    protocols/gtk-shell/gtkshell.cpp
    protocols/gtk-shell/gtksurface.cpp
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)
    set(SOURCES ${SOURCES} bufferattacher.cpp)
endif()

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
        Qt5::Compositor
        Wayland::Client
        Wayland::Server
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
        AbstractPlugin
        ApplicationManager
        Compositor
        CompositorSettings
        ClientWindow
        HomeApplication
        KeyBinding
        KeyBindings
        Output
        OutputWindow
        WindowView
        ShellWindow
    PREFIX
        Server
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandServer_HEADERS
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
        abstractplugin.h
        applicationmanager.h
        compositor.h
        clientwindow.h
        homeapplication.h
        keybinding.h
        output.h
        outputwindow.h
        windowview.h
        shellwindow.h
    DESTINATION
        ${GREENISLAND_INCLUDEDIR}/server
    COMPONENT
        Devel
)
