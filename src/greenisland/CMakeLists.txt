include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Compositor_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(
    ${Qt5Core_DEFINITIONS}
    ${Qt5Gui_DEFINITIONS}
    ${Qt5Quick_DEFINITIONS}
    ${Qt5Compositor_DEFINITIONS}
    -DQT_COMPOSITOR_QUICK
)

set(SOURCES
    main.cpp
    compositor.cpp
)

qt5_add_resources(RESOURCES greenisland.qrc)

add_executable(greenisland ${SOURCES} ${RESOURCES})
set_target_properties(greenisland PROPERTIES
    COMPILE_FLAGS ${Qt5Compositor_EXECUTABLE_COMPILE_FLAGS}
    LINK_FLAGS -L${GLESV2_LIBDIR}
)
target_link_libraries(greenisland
    ${Qt5Quick_LIBRARIES}
    ${Qt5Compositor_LIBRARIES}
    ${VibeCore_LIBRARIES}
    ${GLESV2_LIBRARIES}
    GreenIsland
)

install(TARGETS greenisland DESTINATION ${CMAKE_INSTALL_BINDIR})
