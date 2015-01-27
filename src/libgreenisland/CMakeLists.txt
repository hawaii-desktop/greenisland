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
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

set(SOURCES
    applicationmanager.cpp
    clientwindow.cpp
    compositor.cpp
    globalregistry.cpp
    gldebug.cpp
    homeapplication.cpp
    logging.cpp
    output.cpp
    outputwindow.cpp
    quicksurface.cpp
    windowview.cpp
    screenmanager.cpp
    shellwindowview.cpp
    utilities.cpp
    protocols/fullscreen-shell/fullscreenshellclient.cpp
    protocols/plasma/plasmaeffects.cpp
    protocols/plasma/plasmashell.cpp
    protocols/plasma/plasmasurface.cpp
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
)

if(ENABLE_OPENGL AND ${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)
    set(SOURCES ${SOURCES} bufferattacher.cpp)
endif()

ecm_add_qtwayland_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/xdg-shell.xml
    BASENAME xdg-shell
    PREFIX xdg_
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

ecm_add_qtwayland_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/fullscreen-shell.xml
    BASENAME fullscreen-shell
    PREFIX _wl_
)

add_library(GreenIsland SHARED ${SOURCES})
generate_export_header(GreenIsland EXPORT_FILE_NAME greenisland/greenisland_export.h)
add_library(GreenIsland::GreenIsland ALIAS GreenIsland)

target_link_libraries(GreenIsland
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Quick
    Qt5::Compositor
    KF5::Screen
    Wayland::Client
    Wayland::Server
    Wayland::Egl
    EGL::EGL
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    target_link_libraries(GreenIsland ${OPENGL_LIBRARIES})
else()
    target_link_libraries(GreenIsland ${OPENGLES_LIBRARIES})
endif()

if(systemd_FOUND)
    target_link_libraries(GreenIsland ${systemd_LIBRARIES})
endif()

set(GREENISLAND_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland")

target_include_directories(GreenIsland INTERFACE "$<INSTALL_INTERFACE:${GREENISLAND_INCLUDEDIR}>")

set_target_properties(GreenIsland PROPERTIES VERSION   ${PROJECT_VERSION}
                                             SOVERSION ${PROJECT_SOVERSION}
                                             EXPORT_NAME GreenIsland
)

ecm_generate_headers(GreenIsland_CamelCase_HEADERS
  HEADER_NAMES
    ApplicationManager
    Compositor
    ClientWindow
    HomeApplication
    Output
    OutputWindow
    QuickSurface
    WindowView
    ShellWindowView
  PREFIX
    GreenIsland
  REQUIRED_HEADERS GreenIsland_HEADERS
)
install(FILES ${GreenIsland_CamelCase_HEADERS}
        DESTINATION ${GREENISLAND_INCLUDEDIR}/GreenIsland
        COMPONENT Devel)

install(TARGETS GreenIsland EXPORT GreenIslandTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/greenisland/greenisland_export.h
      ${GreenIsland_HEADERS}
      applicationmanager.h
      compositor.h
      clientwindow.h
      homeapplication.h
      output.h
      outputwindow.h
      quicksurface.h
      windowview.h
      shellwindowview.h
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
              "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT GreenIslandTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE GreenIslandTargets.cmake NAMESPACE GreenIsland::)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/greenisland_version.h
        DESTINATION ${GREENISLAND_INCLUDEDIR}/greenisland COMPONENT Devel)
