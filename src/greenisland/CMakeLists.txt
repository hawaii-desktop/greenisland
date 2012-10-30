include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Compositor_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(${Qt5Compositor_DEFINITIONS})

set(SOURCES
    main.cpp
)

add_executable(greenisland ${SOURCES})
set_target_properties(greenisland PROPERTIES
    COMPILE_FLAGS ${Qt5Compositor_EXECUTABLE_COMPILE_FLAGS}
    LINK_FLAGS -L${GLESV2_LIBDIR}
)
target_link_libraries(greenisland
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Compositor_LIBRARIES}
    ${VibeCore_LIBRARIES}
    ${GLESV2_LIBRARIES}
    GreenIsland
)

install(TARGETS greenisland DESTINATION ${CMAKE_INSTALL_BINDIR})
