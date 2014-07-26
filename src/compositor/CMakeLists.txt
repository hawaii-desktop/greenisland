include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

if(systemd_FOUND)
    include_directories(${systemd_INCLUDE_DIRS})
endif()

# Create the config.h header file
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Create the cmakedirs.h header file
configure_file(cmakedirs.h.in ${CMAKE_CURRENT_BINARY_DIR}/cmakedirs.h)

if(USE_OPENGL)
    add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)
endif()

set(SOURCES
    bufferattacher.cpp
    compositor.cpp
    compositorapp.cpp
    logging.cpp
    main.cpp
    screen.cpp
    screenbackend.cpp
    screenmodel.cpp
    utilities.cpp
    backends/fakescreenbackend.cpp
    backends/qscreenbackend.cpp
)

qt5_add_resources(RESOURCES greenisland.qrc)

add_executable(greenisland ${SOURCES} ${RESOURCES})
target_link_libraries(greenisland
    Qt5::Quick
    Qt5::Compositor
)

if(USE_OPENGL)
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
