include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)

set(SOURCES
    bufferattacher.cpp
    compositor.cpp
    globalregistry.cpp
    gldebug.cpp
    logging.cpp
    main.cpp
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

qt5_add_resources(RESOURCES greenisland.qrc)

add_executable(greenisland ${SOURCES} ${RESOURCES})
target_link_libraries(greenisland
    Qt5::Quick
    Qt5::Compositor
    KF5::Screen
    KF5::Plasma
    Wayland::Client
    Wayland::Server
    Wayland::Egl
    EGL::EGL
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    target_link_libraries(greenisland ${OPENGL_LIBRARIES})
else()
    target_link_libraries(greenisland ${OPENGLES_LIBRARIES})
endif()

if(systemd_FOUND)
    set_target_properties(greenisland PROPERTIES LINK_FLAGS -L${systemd_LIBDIR})
    target_link_libraries(greenisland ${systemd_LIBRARIES})
endif()

install(TARGETS greenisland DESTINATION ${BIN_INSTALL_DIR})
