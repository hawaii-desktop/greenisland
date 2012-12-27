# Install QML types
file(GLOB MODULE_FILES "*.qml")
install(FILES ${MODULE_FILES}
    DESTINATION ${CMAKE_INSTALL_QMLDIR}/GreenIsland)
