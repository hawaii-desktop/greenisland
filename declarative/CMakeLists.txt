find_package(Qt5Gui REQUIRED COMPONENTS Private)
find_package(Qt5Qml REQUIRED COMPONENTS Private)
find_package(Qt5Quick REQUIRED COMPONENTS Private)

# Qt 5.5.x doesn't define Qt5Qml and Qt5Quick private
# include directories, so we have to fix it
foreach(_mod Qml Quick)
    foreach(_dir ${Qt5${_mod}_INCLUDE_DIRS})
        set(_path "${_dir}/Qt${_mod}/${Qt5${_mod}_VERSION_STRING}")
        if(EXISTS ${_path})
            list(APPEND Qt5${_mod}_PRIVATE_INCLUDE_DIRS ${_path})
        endif()

        set(_path "${_dir}/Qt${_mod}/${Qt5${_mod}_VERSION_STRING}/Qt${_mod}")
        if(EXISTS ${_path})
            list(APPEND Qt5${_mod}_PRIVATE_INCLUDE_DIRS ${_path})
        endif()
    endforeach()
endforeach()

include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    plugin.cpp
    quickkeybindings.cpp
    fpscounter.cpp
    globalpointertracker.cpp
    localpointertracker.cpp
    pointeritem.cpp
    keybindingsfilter.cpp
    keyeventfilter.cpp
)

add_library(greenislandplugin SHARED ${SOURCES})
target_link_libraries(greenislandplugin
    Qt5::Qml
    Qt5::Quick
    GreenIsland::Compositor
    GreenIsland::Server
)

install(TARGETS greenislandplugin LIBRARY
    DESTINATION ${QML_INSTALL_DIR}/GreenIsland)

set(MODULE_FILES
    qmldir
    WaylandScreen.qml
    WaylandWindow.qml
)
install(FILES ${MODULE_FILES}
    DESTINATION ${QML_INSTALL_DIR}/GreenIsland)
