include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

add_definitions(-DLINUX=1 -DEGL_API_FB=1)

set(SOURCES
    eglfsvivintegration.cpp
    main.cpp
)

add_library(viv SHARED MODULE ${SOURCES})
target_link_libraries(viv
    GreenIsland::Platform
    Wayland::Server
    GAL
)

install(TARGETS viv
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
