include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    eglfswaylandblitter.cpp
    eglfswaylandcontext.cpp
    eglfswaylandinput.cpp
    eglfswaylandintegration.cpp
    eglfswaylandscreen.cpp
    eglfswaylandwindow.cpp
    main.cpp
)

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(SOURCES
    HEADER eglfswaylandlogging.h
    IDENTIFIER GreenIsland::Platform::gLcEglFSWayland
    CATEGORY_NAME greenisland.qpa.wayland
    DEFAULT_SEVERITY Debug
)

add_library(wayland MODULE ${SOURCES})
target_link_libraries(wayland
    GreenIsland::Platform
    GreenIsland::Client
    Wayland::Egl
)

install(TARGETS wayland
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
