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
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/screen
    ${CMAKE_CURRENT_SOURCE_DIR}/surface
    ${CMAKE_CURRENT_SOURCE_DIR}/wayland_wrapper
    ${CMAKE_CURRENT_SOURCE_DIR}/hardware_integration
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

add_definitions(-Wno-deprecated-declarations)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)
endif()

include(GreenIslandConfigExtrasLocal.cmake)
include(GreenIslandMacros.cmake)

set(SOURCES
    abstractplugin.cpp
    abstractcompositor.cpp
    abstractquickcompositor.cpp
    applicationmanager.cpp
    bufferref.cpp
    clientconnection.cpp
    clientwindow.cpp
    compositor.cpp
    compositor_p.cpp
    compositorsettings.cpp
    cursorbufferattacher_p.cpp
    diagnostic_p.cpp
    drag.cpp
    homeapplication.cpp
    keybinding.cpp
    keybindings.cpp
    globalinterface.cpp
    inputdevice.cpp
    inputpanel.cpp
    listener.cpp
    logging.cpp
    mimehelper_p.cpp
    output.cpp
    outputwindow.cpp
    quicksurface.cpp
    region.cpp
    shellwindow.cpp
    utilities.cpp
    windowview.cpp
    xkbhelper_p.cpp
    client/wlclientconnection.cpp
    client/wlcursortheme.cpp
    client/wlpointer.cpp
    client/wlregistry.cpp
    client/wlseat.cpp
    client/wlshmpool.cpp
    plugins/clientbufferintegrationfactory_p.cpp
    plugins/clientbufferintegrationinterface.cpp
    plugins/serverbufferintegrationfactory_p.cpp
    plugins/serverbufferintegrationinterface.cpp
    screen/screenbackend.cpp
    screen/screenconfiguration.cpp
    screen/fakescreenbackend.cpp
    screen/nativescreenbackend.cpp
    screen/screenmanager.cpp
    surface/surface.cpp
    surface/surfacebuffer.cpp
    surface/surfaceevent.cpp
    surface/surfaceinterface.cpp
    surface/surfaceitem.cpp
    surface/surfaceview.cpp
    protocols/fullscreen-shell/fullscreenshellclient.cpp
    protocols/greenisland/greenislandapps.cpp
    protocols/greenisland/greenislandwindow.cpp
    protocols/greenisland/greenislandwindows.cpp
    protocols/greenisland/greenislandrecorder.cpp
    protocols/greenisland/greenislandscreenshooter.cpp
    protocols/gtk-shell/gtkshell.cpp
    protocols/gtk-shell/gtksurface.cpp
    protocols/qt/qthardwareintegration.cpp
    protocols/qt/qttouchextension.cpp
    protocols/qt/qtwindowmanager.cpp
    protocols/wayland/wlsubcompositor.cpp
    protocols/wayland/wlsubsurface.cpp
    protocols/wl-shell/wlshell.cpp
    protocols/wl-shell/wlshellsurface.cpp
    protocols/wl-shell/wlshellsurfacegrabber.cpp
    protocols/wl-shell/wlshellsurfacepopupgrabber.cpp
    protocols/wl-shell/wlshellsurfacemovegrabber.cpp
    protocols/wl-shell/wlshellsurfaceresizegrabber.cpp
    protocols/xdg-shell/xdgshell.cpp
    protocols/xdg-shell/xdgsurface.cpp
    protocols/xdg-shell/xdgsurfacegrabber.cpp
    protocols/xdg-shell/xdgsurfacemovegrabber.cpp
    protocols/xdg-shell/xdgsurfaceresizegrabber.cpp
    protocols/xdg-shell/xdgpopup.cpp
    protocols/xdg-shell/xdgpopupgrabber.cpp
    wayland_wrapper/qwlcompositor.cpp
    wayland_wrapper/qwldatadevice.cpp
    wayland_wrapper/qwldatadevicemanager.cpp
    wayland_wrapper/qwldataoffer.cpp
    wayland_wrapper/qwldatasource.cpp
    wayland_wrapper/qwldisplay.cpp
    wayland_wrapper/qwlextendedsurface.cpp
    wayland_wrapper/qwlinputdevice.cpp
    wayland_wrapper/qwlinputmethodcontext.cpp
    wayland_wrapper/qwlinputmethod.cpp
    wayland_wrapper/qwlinputpanelsurface.cpp
    wayland_wrapper/qwlkeyboard.cpp
    wayland_wrapper/qwlpointer.cpp
    wayland_wrapper/qwlqtkey.cpp
    wayland_wrapper/qwlsubsurface.cpp
    wayland_wrapper/qwlsurface.cpp
    wayland_wrapper/qwltextinput.cpp
    wayland_wrapper/qwltextinputmanager.cpp
    wayland_wrapper/qwltouch.cpp
)

# Wayland server-side protocols
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/wayland.xml
    BASENAME wayland
    PREFIX wl_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/weston/input-method.xml
    BASENAME input-method
    PREFIX wl_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/weston/text.xml
    BASENAME text
    PREFIX wl_
)

# XDG server-side protocols
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/xdg-shell.xml
    BASENAME xdg-shell
    PREFIX xdg_
)

# Qt server-side protocols
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/touch-extension.xml
    BASENAME touch-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/surface-extension.xml
    BASENAME surface-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/sub-surface-extension.xml
    BASENAME sub-surface-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/qtkey-extension.xml
    BASENAME qtkey-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/windowmanager.xml
    BASENAME windowmanager
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/server-buffer-extension.xml
    BASENAME server-buffer-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/hardware-integration.xml
    BASENAME hardware-integration
    PREFIX qt_
)

# GreenIsland server-side protocols
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

# Gtk+ server-side protocols
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/gtk/gtk-shell.xml
    BASENAME gtk
    PREFIX gtk_
)

# Wayland client-side protocols
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

add_library(GreenIsland SHARED ${SOURCES})
generate_export_header(GreenIsland EXPORT_FILE_NAME greenisland/greenisland_export.h)
add_library(GreenIsland::GreenIsland ALIAS GreenIsland)

if(xkbcommon_FOUND)
    include_directories(${xkbcommon_INCLUDE_DIRS})
    add_definitions(-DHAVE_XKBCOMMON)
    target_link_libraries(GreenIsland PRIVATE ${xkbcommon_LIBRARIES})
endif()

target_link_libraries(GreenIsland
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
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

ecm_generate_headers(GreenIsland_CAMELCASE_HEADERS
  HEADER_NAMES
    AbstractPlugin
    AbstractCompositor
    AbstractQuickCompositor
    ApplicationManager
    BufferRef
    Compositor
    CompositorSettings
    ClientConnection
    ClientWindow
    Drag
    GlobalInterface
    HomeApplication
    InputDevice
    InputPanel
    KeyBinding
    KeyBindings
    Listener
    Output
    OutputWindow
    QuickSurface
    Region
    ShellWindow
    WindowView
  PREFIX
    GreenIsland
  REQUIRED_HEADERS
    GreenIsland_HEADERS
)

ecm_generate_headers(GreenIsland_CAMELCASE_HEADERS
  HEADER_NAMES
    ClientBufferIntegrationInterface
    ServerBufferIntegrationInterface
  PREFIX
    GreenIsland
  REQUIRED_HEADERS
    GreenIsland_HEADERS
  RELATIVE
    plugins
)

ecm_generate_headers(GreenIsland_CAMELCASE_HEADERS
  HEADER_NAMES
    Surface
    SurfaceBuffer
    SurfaceEvent
    SurfaceInterface
    SurfaceItem
    SurfaceView
  PREFIX
    GreenIsland
  REQUIRED_HEADERS
    GreenIsland_HEADERS
  RELATIVE
    surface
)

install(FILES ${GreenIsland_CAMELCASE_HEADERS}
        DESTINATION ${GREENISLAND_INCLUDEDIR}/GreenIsland
        COMPONENT Devel)

install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/greenisland/greenisland_export.h
      ${GreenIsland_HEADERS}
    DESTINATION
      ${GREENISLAND_INCLUDEDIR}/greenisland
    COMPONENT
      Devel
)

install(TARGETS GreenIsland EXPORT GreenIslandTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

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
