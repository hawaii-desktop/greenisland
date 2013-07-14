include_directories(
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(-DQT_PLUGIN)

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
    popupwindow.cpp
)

add_library(greenislandqmlplugin SHARED ${SOURCES})
qt5_use_modules(greenislandqmlplugin Qml Quick Widgets Xdg AccountsService)
target_link_libraries(greenislandqmlplugin
    GreenIsland
)

install(TARGETS greenislandqmlplugin LIBRARY
    DESTINATION ${QT_IMPORTS_DIR}/GreenIsland)
install(FILES qmldir
    DESTINATION ${QT_IMPORTS_DIR}/GreenIsland)

add_subdirectory(qml)
