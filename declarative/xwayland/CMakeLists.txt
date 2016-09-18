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


# All the required XCB components
#find_package(XCB
#    REQUIRED COMPONENTS
#        DAMAGE
#        SYNC
#        RANDR
#        KEYSYMS
#        IMAGE
#        SHM
#        GLX
#    OPTIONAL_COMPONENTS
#        ICCCM
#        CURSOR
#)
find_package(XCB
    REQUIRED COMPONENTS
        XCB
        XFIXES
        CURSOR
        COMPOSITE
        RENDER
        SHAPE
)
set_package_properties(XCB PROPERTIES TYPE REQUIRED)

if(NOT X11_FOUND)
    message(FATAL_ERROR "X11 is required to build XWayland support")
endif()
if(NOT X11_Xcursor_FOUND)
    message(FATAL_ERROR "Xcursor is required to build XWayland support")
endif()


include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/../../headers"
    "${CMAKE_CURRENT_BINARY_DIR}/../../headers/GreenIsland"
    "${CMAKE_CURRENT_BINARY_DIR}/../../headers/GreenIsland/QtWaylandCompositor"
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    plugin.cpp
    xcbatom.cpp
    xcbatoms.cpp
    xcbcursors.cpp
    xcbproperties.cpp
    xcbresources.cpp
    xcbwindow.cpp
    xcbwrapper.cpp
    xwayland.cpp
    xwaylandmanager.cpp
    xwaylandshellsurface.cpp
    xwaylandquickshellintegration.cpp
    xwaylandquickshellsurfaceitem.cpp
    xwaylandserver.cpp
    sigwatch.cpp
)

add_library(GreenIslandXWaylandPlugin SHARED ${SOURCES})
add_library(GreenIsland::XWaylandPlugin ALIAS GreenIslandXWaylandPlugin)
target_link_libraries(GreenIslandXWaylandPlugin
    Qt5::Qml
    Qt5::Quick
    GreenIsland::Compositor
    GreenIsland::Server
    XCB::XCB
    XCB::XFIXES
    XCB::CURSOR
    XCB::COMPOSITE
    XCB::RENDER
    XCB::SHAPE
    ${X11_Xcursor_LIB}
)

install(TARGETS GreenIslandXWaylandPlugin LIBRARY
    DESTINATION ${QML_INSTALL_DIR}/GreenIsland/XWayland)

set(MODULE_FILES
    qmldir
    XWaylandWindowChrome.qml
)
install(FILES ${MODULE_FILES}
    DESTINATION ${QML_INSTALL_DIR}/GreenIsland/XWayland)
