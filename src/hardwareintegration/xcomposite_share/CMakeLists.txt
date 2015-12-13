include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerConfigExtrasLocal.cmake)
include(${CMAKE_SOURCE_DIR}/src/server/GreenIslandServerMacros.cmake)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_BINARY_DIR}/src/waylandcompositor
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    xcompositebuffer.cpp
    xcompositehandler.cpp
)

greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/wayland.xml
    BASENAME wayland
    PREFIX wl_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/xcomposite.xml
    BASENAME xcomposite
    PREFIX qt_
)

add_library(GreenIslandXComposite STATIC ${SOURCES})
add_library(GreenIsland::XComposite ALIAS GreenIslandXComposite)
target_link_libraries(GreenIslandXComposite
                      GreenIsland::Compositor
                      Wayland::Server)
