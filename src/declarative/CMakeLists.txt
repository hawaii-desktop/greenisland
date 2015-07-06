include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/src/libgreenisland
)

set(SOURCES
    plugin.cpp
    fpscounter.cpp
)

add_library(greenislandplugin SHARED ${SOURCES})
target_link_libraries(greenislandplugin Qt5::Qml Qt5::Quick GreenIsland)

install(TARGETS greenislandplugin LIBRARY
    DESTINATION ${QML_INSTALL_DIR}/GreenIsland)

set(MODULE_FILES
    qmldir
    ContrastEffect.qml
    SurfaceRenderer.qml
)
set(DESKTOP_MODULE_FILES
    Desktop/qmldir
    Desktop/PopupWindow.qml
    Desktop/TopLevelWindow.qml
    Desktop/TransientWindow.qml
    Desktop/WindowAnimation.qml
    Desktop/WindowListModel.qml
    Desktop/WindowManager.qml
    Desktop/WindowPreview.qml
    Desktop/WindowWrapper.qml
    Desktop/Workspace.qml
)
install(FILES ${MODULE_FILES} DESTINATION ${QML_INSTALL_DIR}/GreenIsland)
install(FILES ${DESKTOP_MODULE_FILES} DESTINATION ${QML_INSTALL_DIR}/GreenIsland/Desktop)
