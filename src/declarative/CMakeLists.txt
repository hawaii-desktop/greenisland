include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/headers
)

set(SOURCES
    plugin.cpp
    fpscounter.cpp
)

add_library(greenislandplugin SHARED ${SOURCES})
qt5_use_modules(greenislandplugin Qml)

install(TARGETS greenislandplugin LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/hawaii/qml/GreenIsland)

set(MODULE_FILES
    qmldir
    ClientWindowWrapper.qml
    ContrastEffect.qml
    SurfaceRenderer.qml
    WindowItem.qml
    WindowWrapper.qml
)
install(FILES ${MODULE_FILES}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/hawaii/qml/GreenIsland)
