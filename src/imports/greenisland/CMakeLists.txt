include_directories(
    ${Qt5Qml_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${VibeCore_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(
    ${Qt5Qml_DEFINITIONS}
    ${Qt5Quick_DEFINITIONS}
    ${Qt5Widgets_DEFINITIONS}
    -DQT_PLUGIN
)

set(SOURCES
    main.cpp
    enums.cpp
    appsmodel.cpp
    appchoosercategories.cpp
    indicatorsmodel.cpp
    launcherapplication.cpp
    launcherdropitem.cpp
    launcheritem.cpp
    launchermodel.cpp
    launcherurl.cpp
    listaggregatormodel.cpp
)

add_library(greenislandqmlplugin SHARED ${SOURCES})
target_link_libraries(greenislandqmlplugin
    ${Qt5Qml_LIBRARIES}
    ${Qt5Quick_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${VibeCore_LIBRARIES}
    GreenIsland
)

install(TARGETS greenislandqmlplugin LIBRARY DESTINATION ${CMAKE_INSTALL_QMLDIR}/GreenIsland)
install(FILES qmldir DESTINATION ${CMAKE_INSTALL_QMLDIR}/GreenIsland)

add_subdirectory(qml)
