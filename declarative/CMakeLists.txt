include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

set(SOURCES
    plugin.cpp
    fpscounter.cpp
)

add_library(greenislandplugin SHARED ${SOURCES})
target_link_libraries(greenislandplugin Qt5::Qml Qt5::Quick GreenIsland::Server)

install(TARGETS greenislandplugin LIBRARY
    DESTINATION ${QML_INSTALL_DIR}/GreenIsland)

set(MODULE_FILES
    qmldir
    ContrastEffect.qml
    SurfaceRenderer.qml
)
install(FILES ${MODULE_FILES}
    DESTINATION ${QML_INSTALL_DIR}/GreenIsland)
