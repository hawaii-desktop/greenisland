include("${CMAKE_CURRENT_SOURCE_DIR}/../../server/GreenIslandServerConfigExtrasLocal.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../server/GreenIslandServerMacros.cmake")

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/.."
    "${CMAKE_CURRENT_BINARY_DIR}/../waylandcompositor"
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers"
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers/GreenIsland"
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    xcompositebuffer.cpp
    xcompositehandler.cpp
)

greenisland_add_server_protocol(SOURCES
    PROTOCOL "${CMAKE_CURRENT_SOURCE_DIR}/../../../data/protocols/wayland/wayland.xml"
    BASENAME wayland
    PREFIX wl_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL "${CMAKE_CURRENT_SOURCE_DIR}/../../../data/protocols/qt/xcomposite.xml"
    BASENAME xcomposite
    PREFIX qt_
)

add_library(GreenIslandXComposite STATIC ${SOURCES})
add_library(GreenIsland::XComposite ALIAS GreenIslandXComposite)
target_link_libraries(GreenIslandXComposite
                      GreenIsland::Compositor
                      Wayland::Server)
