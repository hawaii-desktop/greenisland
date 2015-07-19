include(GenerateExportHeader)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX GREENISLAND
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/greenisland_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfigVersion.cmake"
                  SOVERSION ${PROJECT_SOVERSION})

include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/screen
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

add_definitions(-Wno-deprecated-declarations)

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

ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland.xml
    BASENAME greenisland
    PREFIX greenisland_
)
ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland-recorder.xml
    BASENAME greenisland-recorder
    PREFIX greenisland_
)
ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland-screenshooter.xml
    BASENAME greenisland-screenshooter
    PREFIX greenisland_
)
ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/xdg-shell.xml
    BASENAME xdg-shell
    PREFIX xdg_
)
ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/gtk-shell.xml
    BASENAME gtk
    PREFIX gtk_
)

ecm_add_qtwayland_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland.xml
    BASENAME wayland
    PREFIX wl_
)
ecm_add_qtwayland_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/fullscreen-shell.xml
    BASENAME fullscreen-shell
    PREFIX _wl_
)

add_library(GreenIsland SHARED ${SOURCES})
generate_export_header(GreenIsland EXPORT_FILE_NAME greenisland/greenisland_export.h)
add_library(GreenIsland::GreenIsland ALIAS GreenIsland)

target_link_libraries(GreenIsland
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
        target_link_libraries(GreenIsland PRIVATE ${OPENGL_LIBRARIES})
    else()
        target_link_libraries(GreenIsland PRIVATE ${OPENGLES_LIBRARIES})
    endif()
endif()

if(systemd_FOUND)
    target_link_libraries(GreenIsland PRIVATE ${systemd_LIBRARIES})
endif()

set(GREENISLAND_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland")

target_include_directories(GreenIsland INTERFACE "$<INSTALL_INTERFACE:${GREENISLAND_INCLUDEDIR}>")

set_target_properties(GreenIsland PROPERTIES VERSION   ${PROJECT_VERSION}
                                             SOVERSION ${PROJECT_SOVERSION}
                                             EXPORT_NAME GreenIsland
)

ecm_generate_headers(GreenIsland_CamelCase_HEADERS
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
    GreenIsland
  REQUIRED_HEADERS GreenIsland_HEADERS
)
install(FILES ${GreenIsland_CamelCase_HEADERS}
        DESTINATION ${GREENISLAND_INCLUDEDIR}/GreenIsland
        COMPONENT Devel)

install(TARGETS GreenIsland EXPORT GreenIslandTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/greenisland/greenisland_export.h
      ${GreenIsland_HEADERS}
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
      ${GREENISLAND_INCLUDEDIR}/greenisland
    COMPONENT
      Devel
)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/GreenIsland")

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/GreenIslandConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfig.cmake"
                                  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                                  )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfig.cmake"
              "${CMAKE_CURRENT_SOURCE_DIR}/GreenIslandConfigExtras.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfigVersion.cmake"
              "${CMAKE_CURRENT_SOURCE_DIR}/GreenIslandMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT GreenIslandTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE GreenIslandTargets.cmake NAMESPACE GreenIsland::)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/greenisland_version.h
        DESTINATION ${GREENISLAND_INCLUDEDIR}/greenisland COMPONENT Devel)
