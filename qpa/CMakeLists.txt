include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/src/platform
)

set(SOURCES
    main.cpp
)

add_library(GreenIslandEglFS MODULE ${SOURCES})
target_link_libraries(GreenIslandEglFS
    Qt5::Core
    Qt5::Gui
    GreenIslandPlatform
)

install(TARGETS GreenIslandEglFS DESTINATION ${PLUGIN_INSTALL_DIR}/platforms)
