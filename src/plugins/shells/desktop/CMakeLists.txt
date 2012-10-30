include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Compositor_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(
    ${Qt5Quick_DEFINITIONS}
    ${Qt5Widgets_DEFINITIONS}
    ${Qt5Compositor_DEFINITIONS}
    -DQT_PLUGIN
    -DQT_COMPOSITOR_QUICK
)

set(SOURCES
    main.cpp
    desktopshell.cpp
)

qt5_add_resources(RESOURCES desktop-shell.qrc)

add_library(desktop SHARED ${SOURCES} ${RESOURCES})
target_link_libraries(desktop
    ${Qt5Quick_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${VibeCore_LIBRARIES}
    GreenIsland
)

install(TARGETS desktop DESTINATION ${CMAKE_INSTALL_PLUGINSDIR}/greenisland/shells)
