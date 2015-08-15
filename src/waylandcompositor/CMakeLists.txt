include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${CMAKE_CURRENT_SOURCE_DIR}/compositor_api
    ${CMAKE_CURRENT_SOURCE_DIR}/wayland_wrapper
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

include(../GreenIslandConfigExtrasLocal.cmake)
include(../GreenIslandMacros.cmake)

set(SOURCES
    shared/qwaylandmimehelper.cpp
    wayland_wrapper/qwlcompositor.cpp
    wayland_wrapper/qwldatadevice.cpp
    wayland_wrapper/qwldatadevicemanager.cpp
    wayland_wrapper/qwldataoffer.cpp
    wayland_wrapper/qwldatasource.cpp
    wayland_wrapper/qwldisplay.cpp
    wayland_wrapper/qwlextendedsurface.cpp
    wayland_wrapper/qwlinputdevice.cpp
    wayland_wrapper/qwlinputmethodcontext.cpp
    wayland_wrapper/qwlinputmethod.cpp
    wayland_wrapper/qwlinputpanel.cpp
    wayland_wrapper/qwlinputpanelsurface.cpp
    wayland_wrapper/qwlkeyboard.cpp
    wayland_wrapper/qwllistener.cpp
    wayland_wrapper/qwloutput.cpp
    wayland_wrapper/qwlpointer.cpp
    wayland_wrapper/qwlqtkey.cpp
    wayland_wrapper/qwlqttouch.cpp
    wayland_wrapper/qwlregion.cpp
    wayland_wrapper/qwlsubsurface.cpp
    wayland_wrapper/qwlsurfacebuffer.cpp
    wayland_wrapper/qwlsurface.cpp
    wayland_wrapper/qwltextinput.cpp
    wayland_wrapper/qwltextinputmanager.cpp
    wayland_wrapper/qwltouch.cpp
    hardware_integration/qwlclientbufferintegration.cpp
    hardware_integration/qwlclientbufferintegrationfactory.cpp
    hardware_integration/qwlclientbufferintegrationplugin.cpp
    hardware_integration/qwlhwintegration.cpp
    hardware_integration/qwlserverbufferintegration.cpp
    hardware_integration/qwlserverbufferintegrationfactory.cpp
    hardware_integration/qwlserverbufferintegrationplugin.cpp
    compositor_api/qwaylandbufferref.cpp
    compositor_api/qwaylandclient.cpp
    compositor_api/qwaylandcompositor.cpp
    compositor_api/qwaylanddrag.cpp
    compositor_api/qwaylandglobalinterface.cpp
    compositor_api/qwaylandinput.cpp
    compositor_api/qwaylandinputpanel.cpp
    compositor_api/qwaylandoutput.cpp
    compositor_api/qwaylandquickcompositor.cpp
    compositor_api/qwaylandquickoutput.cpp
    compositor_api/qwaylandquicksurface.cpp
    compositor_api/qwaylandsurface.cpp
    compositor_api/qwaylandsurfaceinterface.cpp
    compositor_api/qwaylandsurfaceitem.cpp
    compositor_api/qwaylandsurfaceview.cpp
    compositor_api/qwaylandtexturebufferattacher.cpp
    windowmanagerprotocol/waylandwindowmanagerintegration.cpp
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    add_definitions(-DQT_COMPOSITOR_WAYLAND_GL)
endif()

greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/surface-extension.xml
    BASENAME surface-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/touch-extension.xml
    BASENAME touch-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/qtkey-extension.xml
    BASENAME qtkey-extension
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/windowmanager.xml
    BASENAME windowmanager
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/qt/hardware-integration.xml
    BASENAME hardware-integration
    PREFIX qt_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/wayland/wayland.xml
    BASENAME wayland
    PREFIX wl_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/weston/input-method.xml
    BASENAME input-method
    PREFIX wl_
)
greenisland_add_server_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/weston/text.xml
    BASENAME text
    PREFIX wl_
)

add_library(GreenIslandCompositor SHARED ${SOURCES})
add_library(GreenIsland::Compositor ALIAS GreenIslandCompositor)

target_link_libraries(GreenIslandCompositor
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
        Wayland::Server
)

if(xkbcommon_FOUND)
    include_directories(${xkbcommon_INCLUDE_DIRS})
    add_definitions(-DHAVE_XKBCOMMON)
    target_link_libraries(GreenIslandCompositor PRIVATE ${xkbcommon_LIBRARIES})
endif()

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    target_link_libraries(GreenIslandCompositor PUBLIC ${OPENGL_LIBRARIES})
else()
    target_link_libraries(GreenIslandCompositor PUBLIC ${OPENGLES_LIBRARIES})
endif()

target_include_directories(
    GreenIslandCompositor
        INTERFACE
            "$<INSTALL_INTERFACE:${HAWAII_INCLUDEDIR}>"
            "$<INSTALL_INTERFACE:${GREENISLAND_INCLUDEDIR}>"
)

set_target_properties(GreenIslandCompositor PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_SOVERSION}
    EXPORT_NAME Compositor
)

ecm_generate_headers(GreenIslandCompositor_CamelCase_HEADERS
    HEADER_NAMES
        QWaylandBufferRef
        QWaylandClient
        QWaylandCompositor
        QWaylandDrag
        QWaylandGlobalInterface
        QWaylandInput
        QWaylandInputPanel
        QWaylandOutput
        QWaylandQuickCompositor
        QWaylandQuickOutput
        QWaylandQuickSurface
        QWaylandSurface
        QWaylandSurfaceInterface
        QWaylandSurfaceItem
        QWaylandSurfaceView
    PREFIX
        Compositor
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandCompositor_HEADERS
    RELATIVE
        compositor_api
)

ecm_generate_headers(GreenIslandCompositor_CamelCase_HEADERS
    HEADER_NAMES
        QWaylandExport
    PREFIX
        Compositor
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandCompositor_HEADERS
    RELATIVE
        global
)

install(TARGETS GreenIslandCompositor
        EXPORT GreenIslandTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${GreenIslandCompositor_CamelCase_HEADERS}
        DESTINATION ${GREENISLAND_INCLUDEDIR}/Compositor
        COMPONENT Devel)

install(
    FILES
        ${GreenIslandCompositor_HEADERS}
        compositor_api/qwaylandbufferref.h
        compositor_api/qwaylandclient.h
        compositor_api/qwaylandcompositor.h
        compositor_api/qwaylanddrag.h
        compositor_api/qwaylandglobalinterface.h
        compositor_api/qwaylandinput.h
        compositor_api/qwaylandinputpanel.h
        compositor_api/qwaylandoutput.h
        compositor_api/qwaylandquickcompositor.h
        compositor_api/qwaylandquickoutput.h
        compositor_api/qwaylandquicksurface.h
        compositor_api/qwaylandsurface.h
        compositor_api/qwaylandsurfaceinterface.h
        compositor_api/qwaylandsurfaceitem.h
        compositor_api/qwaylandsurface_p.h
        compositor_api/qwaylandsurfaceview.h
        compositor_api/qwaylandtexturebufferattacher_p.h
        global/qwaylandexport.h
    DESTINATION
        ${GREENISLAND_INCLUDEDIR}/compositor
    COMPONENT
        Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME GreenIslandCompositor
                      LIB_NAME GreenIslandCompositor
                      DEPS "core gui quick"
                      FILENAME_VAR PRI_FILENAME
                      INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}")
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

include(ECMGeneratePkgConfigFile)
ecm_generate_pkgconfig_file(BASE_NAME GreenIslandCompositor
                            LIB_NAME GreenIslandCompositor
                            DEPS "Qt5Core Qt5Gui Qt5Quick"
                            FILENAME_VAR PKGCONFIG_FILENAME
                            INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}"
                            INSTALL)
