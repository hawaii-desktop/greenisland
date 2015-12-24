include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    eglfsmaliintegration.cpp
    main.cpp
)

add_library(mali SHARED MODULE ${SOURCES})
target_link_libraries(mali
    GreenIsland::Platform
)

install(TARGETS mali
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
