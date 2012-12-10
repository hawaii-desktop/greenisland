include_directories(
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${GLESV2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(
    ${Qt5Gui_DEFINITIONS}
    -DQT_COMPOSITOR_QUICK
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/gitsha1.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gitsha1.cpp" @ONLY
)

set(SOURCES
    main.cpp
    greenisland.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/gitsha1.cpp"
)

add_executable(greenisland ${SOURCES})
set_target_properties(greenisland PROPERTIES
    COMPILE_FLAGS ${Qt5Gui_EXECUTABLE_COMPILE_FLAGS}
    LINK_FLAGS "-L${EGL_LIBDIR} -L${GLESV2_LIBDIR}"
)
target_link_libraries(greenisland
    ${Qt5Gui_LIBRARIES}
    ${VibeCore_LIBRARIES}
    GreenIsland
    ${EGL_LIBRARIES}
    ${GLESV2_LIBRARIES}
)

install(TARGETS greenisland DESTINATION ${CMAKE_INSTALL_BINDIR})
