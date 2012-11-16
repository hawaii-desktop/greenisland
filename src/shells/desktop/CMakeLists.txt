include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(
    ${Qt5Gui_DEFINITIONS}
    ${Qt5Quick_DEFINITIONS}
)

set(SOURCES
    main.cpp
    desktopshell.cpp
    desktopshellintegration.cpp
    shellquickview.cpp
    panelview.cpp
    launcherview.cpp
)

wayland_add_protocol_client(SOURCES ${CMAKE_SOURCE_DIR}/protocol/desktop-shell.xml desktop-extension)

qt5_add_resources(RESOURCES desktop-shell.qrc)

add_executable(greenisland-desktop-shell ${SOURCES} ${RESOURCES})
set_target_properties(greenisland-desktop-shell PROPERTIES
    COMPILE_FLAGS ${Qt5Quick_EXECUTABLE_COMPILE_FLAGS}
    LINK_FLAGS "-L${GLESV2_LIBDIR} -L${WAYLAND_CLIENT_LIBDIR}"
)
target_link_libraries(greenisland-desktop-shell
    ${Qt5Quick_LIBRARIES}
    ${VibeCore_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    GreenIsland
)

install(TARGETS greenisland-desktop-shell DESTINATION ${CMAKE_INSTALL_BINDIR})
