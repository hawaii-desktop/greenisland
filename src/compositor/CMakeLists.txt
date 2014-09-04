include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

# Create the config.h header file
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Create the cmakedirs.h header file
configure_file(cmakedirs.h.in ${CMAKE_CURRENT_BINARY_DIR}/cmakedirs.h)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)
endif()

set(SOURCES
    bufferattacher.cpp
    compositor.cpp
    logging.cpp
    main.cpp
    output.cpp
    outputwindow.cpp
    windowview.cpp
    screenmanager.cpp
    utilities.cpp
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

wayland_add_protocol_client(SOURCES
    ${CMAKE_SOURCE_DIR}/data/protocols/xdg-shell.xml
    xdg-shell
)
wayland_add_protocol_server(SOURCES
    ${CMAKE_SOURCE_DIR}/data/protocols/xdg-shell.xml
    xdg-shell
)
qtwayland_add_protocol_server(SOURCES
    ${CMAKE_SOURCE_DIR}/data/protocols/xdg-shell.xml
    xdg-shell xdg_
)

qt5_add_resources(RESOURCES greenisland.qrc)

add_executable(greenisland ${SOURCES} ${RESOURCES})
target_link_libraries(greenisland
    Qt5::Quick
    Qt5::Compositor
    KF5::Screen
    Wayland::Client
    Wayland::Server
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    set_target_properties(greenisland PROPERTIES LINK_FLAGS -L${GL_LIBDIR})
    target_link_libraries(greenisland ${GL_LIBRARIES})
else()
    set_target_properties(greenisland PROPERTIES LINK_FLAGS -L${EGL_LIBDIR})
    target_link_libraries(greenisland ${EGL_LIBRARIES})
endif()

if(systemd_FOUND)
    set_target_properties(greenisland PROPERTIES LINK_FLAGS -L${systemd_LIBDIR})
    target_link_libraries(greenisland ${systemd_LIBRARIES})
endif()

install(TARGETS greenisland DESTINATION ${CMAKE_INSTALL_BINDIR})
