include_directories(
    ${VibeCore_INCLUDE_DIRS}
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
)

add_library(greenislandqmlplugin SHARED ${SOURCES})
qt5_use_modules(greenislandqmlplugin Qml Quick Widgets AccountsService)
target_link_libraries(greenislandqmlplugin
    ${VibeCore_LIBRARIES}
    GreenIsland
)

install(TARGETS greenislandqmlplugin LIBRARY
    DESTINATION ${CMAKE_INSTALL_QMLDIR}/GreenIsland)
install(FILES qmldir
    DESTINATION ${CMAKE_INSTALL_QMLDIR}/GreenIsland)

add_subdirectory(qml)
