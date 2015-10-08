include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    main.cpp
)

add_library(GreenIslandEglFS MODULE ${SOURCES})
target_link_libraries(GreenIslandEglFS
    Qt5::Core
    Qt5::Gui
    GreenIsland::Platform
)

install(TARGETS GreenIslandEglFS DESTINATION ${PLUGIN_INSTALL_DIR}/platforms)
