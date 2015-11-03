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
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Compositor_PRIVATE_INCLUDE_DIRS}
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

add_library(xwayland MODULE ${SOURCES})
target_link_libraries(xwayland
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Compositor
                      GreenIsland::Server
                      XCB::XCB
                      XCB::XFIXES
                      XCB::CURSOR
                      XCB::COMPOSITE
                      XCB::RENDER
                      XCB::SHAPE
                      ${X11_Xcursor_LIB}
                      Wayland::Server)

install(TARGETS xwayland
    DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/extensions)
