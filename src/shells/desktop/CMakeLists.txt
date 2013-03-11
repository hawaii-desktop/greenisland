include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${POLKITQT-1_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

set(SOURCES
    main.cpp
    desktopshell.cpp
    waylandintegration.cpp
    output.cpp
    background.cpp
    launcher.cpp
    panel.cpp
    appchooser.cpp
)
#    notificationsdaemon.cpp
#    notificationsimage.cpp
#    policykitagent.cpp

wayland_add_protocol_client(SOURCES ${CMAKE_SOURCE_DIR}/protocol/desktop-shell.xml desktop-extension)

qt5_add_resources(RESOURCES desktop-shell.qrc)
#qt5_add_dbus_adaptor(SOURCES ${CMAKE_SOURCE_DIR}/data/org.freedesktop.Notifications.xml
#    notificationsdaemon.h NotificationsDaemon)

add_executable(greenisland-desktop-shell ${SOURCES} ${RESOURCES})
qt5_use_modules(greenisland-desktop-shell Quick Widgets AccountsService)
set_target_properties(greenisland-desktop-shell PROPERTIES
    COMPILE_FLAGS "${Qt5Quick_EXECUTABLE_COMPILE_FLAGS} -rdynamic"
    LINK_FLAGS "-L${GLESV2_LIBDIR} -L${WAYLAND_CLIENT_LIBDIR} -L${POLKITQT-1_LIB_DIR}"
)
target_link_libraries(greenisland-desktop-shell
    ${VibeCore_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${POLKITQT-1_CORE_LIBRARY}
    ${POLKITQT-1_AGENT_LIBRARY}
    GreenIsland
)

install(TARGETS greenisland-desktop-shell
    DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
