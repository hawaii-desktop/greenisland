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

find_package(X11)
set_package_properties(X11 PROPERTIES
                       PURPOSE "Required for cursors support of xwayland"
                       TYPE OPTIONAL)
if(NOT X11_Xcursor_FOUND)
    message(FATAL_ERROR "Xcursor is required by xwayland")
endif()

include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/libgreenisland
    ${CMAKE_BINARY_DIR}/src/libgreenisland
    ${CMAKE_BINARY_DIR}/src/libgreenisland/greenisland
)

set(SOURCES
    xcbatom.cpp
    xcbatoms.cpp
    xcbcursors.cpp
    xcbwindow.cpp
    xcbwrapper.cpp
    xcbresources.cpp
    xwayland.cpp
    xwaylandmanager.cpp
    xwaylandplugin.cpp
    xwaylandwindow.cpp
    xwaylandserver.cpp
    xwaylandsurface.cpp
    sigwatch.cpp
)

add_library(xwayland SHARED ${SOURCES})
set_target_properties(xwayland PROPERTIES PREFIX "")
target_link_libraries(xwayland
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Compositor
                      GreenIsland::GreenIsland
                      XCB::XCB
                      XCB::XFIXES
                      XCB::CURSOR
                      XCB::COMPOSITE
                      XCB::RENDER
                      XCB::SHAPE
                      ${X11_Xcursor_LIB}
                      Wayland::Server)

install(TARGETS xwayland
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland)
