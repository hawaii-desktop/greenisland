# udev
find_package(UDev)
set_package_properties(UDev PROPERTIES
    DESCRIPTION "Linux device library"
    URL "http://www.freedesktop.org/software/systemd/libudev/"
    TYPE REQUIRED
    PURPOSE "Required to access devices from the QPA plugin")

# libinput
find_package(Libinput 0.12)
set_package_properties(Libinput PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for input handling from the QPA plugin")

# gio
pkg_check_modules(gio gio-2.0 REQUIRED)

# Qt5PlatformSupport
find_package(Qt5PlatformSupport REQUIRED)
find_package(Freetype REQUIRED)
set_package_properties(Freetype PROPERTIES
    DESCRIPTION "A font rendering engine"
    URL "http://www.freetype.org"
    TYPE REQUIRED
    PURPOSE "Required by the QPA plugin")
find_package(Fontconfig REQUIRED)
set_package_properties(Fontconfig PROPERTIES
    DESCRIPTION "Font access configuration library"
    URL "http://www.freedesktop.org/wiki/Software/fontconfig"
    TYPE REQUIRED
    PURPOSE "Required by the QPA plugin")

set(SOURCES
    logging.cpp
    deviceintegration/deviceintegration_p.cpp
    deviceintegration/egldeviceintegration.cpp
    deviceintegration/eglfscontext.cpp
    deviceintegration/eglfscursor.cpp
    deviceintegration/eglfsintegration.cpp
    deviceintegration/eglfsoffscreenwindow.cpp
    deviceintegration/eglfsscreen.cpp
    deviceintegration/eglfswindow.cpp
    deviceintegration/eglfsxkb.cpp
    eglconvenience/eglconvenience.cpp
    eglconvenience/eglstreamconvenience.cpp
    udev/udev.cpp
    udev/udevdevice.cpp
    udev/udevenumerate.cpp
    udev/udevmonitor.cpp
    logind/logind.cpp
    logind/vthandler.cpp
    libinput/libinputgesture.cpp
    libinput/libinputhandler.cpp
    libinput/libinputmanager_p.cpp
    libinput/libinputkeyboard.cpp
    libinput/libinputpointer.cpp
    libinput/libinputtouch.cpp
    platformcompositor/openglcompositorbackingstore.cpp
    platformcompositor/openglcompositor.cpp
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    set(SOURCES
        ${SOURCES}
        eglconvenience/eglpbuffer.cpp
        eglconvenience/eglplatformcontext.cpp
    )
endif()

qt5_add_resources(SOURCES deviceintegration/cursor.qrc)

add_library(GreenIslandPlatform SHARED ${SOURCES})
generate_export_header(GreenIslandPlatform EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/headers/GreenIsland/platform/greenislandplatform_export.h)
add_library(GreenIsland::Platform ALIAS GreenIslandPlatform)

target_include_directories(GreenIslandPlatform
    PUBLIC
        ${UDEV_INCLUDE_DIR}
        ${xkbcommon_INCLUDE_DIRS}
        ${gio_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_BINARY_DIR}/headers
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
        ${Qt5Core_PRIVATE_INCLUDE_DIRS}
        ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(GreenIslandPlatform
    PUBLIC
        Qt5::Core
        Qt5::DBus
        Qt5::Gui
        EGL::EGL
        ${UDEV_LIBS}
        ${xkbcommon_LIBRARIES}
        ${gio_LIBRARIES}
    PRIVATE
        Libinput::Libinput
        ${FONTCONFIG_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        Qt5PlatformSupport::Qt5PlatformSupport
)

target_compile_definitions(GreenIslandPlatform
    PUBLIC
        -DMESA_EGL_NO_X11_HEADERS
        -DQ_FONTCONFIGDATABASE
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    target_link_libraries(GreenIslandPlatform PUBLIC ${OPENGL_LIBRARIES})
else()
    target_link_libraries(GreenIslandPlatform PUBLIC ${OPENGLES_LIBRARIES})
endif()

target_include_directories(
    GreenIslandPlatform
        INTERFACE
	    "$<INSTALL_INTERFACE:${HAWAII_INCLUDEDIR}>"
	    "$<INSTALL_INTERFACE:${GREENISLAND_INCLUDEDIR}>"
)

set_target_properties(GreenIslandPlatform PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_SOVERSION}
    EXPORT_NAME Platform
)

ecm_generate_headers(GreenIslandPlatform_CAMELCASE_HEADERS
    HEADER_NAMES
        EGLDeviceIntegration
        EglFSContext
        EglFSCursor
        EglFSIntegration
        EglFSOffscreenWindow
        EglFSScreen
        EglFSWindow
        EglFSXkb
    PREFIX
        Platform
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandPlatform_HEADERS
    RELATIVE
        deviceintegration
)

ecm_generate_headers(GreenIslandPlatform_CAMELCASE_HEADERS
    HEADER_NAMES
        EGLConvenience
        EGLPbuffer
        EGLPlatformContext
        EGLStreamConvenience
    PREFIX
        Platform
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandPlatform_HEADERS
    RELATIVE
        eglconvenience
)

ecm_generate_headers(GreenIslandPlatform_CAMELCASE_HEADERS
    HEADER_NAMES
        Udev
        UdevDevice
        UdevEnumerate
        UdevMonitor
    PREFIX
        Platform
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandPlatform_HEADERS
    RELATIVE
        udev
)

ecm_generate_headers(GreenIslandPlatform_CAMELCASE_HEADERS
    HEADER_NAMES
        Logind
        VtHandler
    PREFIX
        Platform
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandPlatform_HEADERS
    RELATIVE
        logind
)

ecm_generate_headers(GreenIslandPlatform_CAMELCASE_HEADERS
    HEADER_NAMES
        LibInputHandler
    PREFIX
        Platform
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandPlatform_HEADERS
    RELATIVE
       libinput
)

ecm_generate_headers(GreenIslandPlatform_CAMELCASE_HEADERS
    HEADER_NAMES
        OpenGLCompositor
        OpenGLCompositorBackingStore
    PREFIX
        Platform
    OUTPUT_DIR
        ${CMAKE_BINARY_DIR}/headers/GreenIsland
    REQUIRED_HEADERS
        GreenIslandPlatform_HEADERS
    RELATIVE
       platformcompositor
)

install(FILES ${GreenIslandPlatform_CAMELCASE_HEADERS}
        DESTINATION ${GREENISLAND_INCLUDEDIR}/Platform
        COMPONENT Devel)

install(
    FILES
        ${CMAKE_BINARY_DIR}/headers/GreenIsland/platform/greenislandplatform_export.h
        ${GreenIslandPlatform_HEADERS}
    DESTINATION
        ${GREENISLAND_INCLUDEDIR}/platform
    COMPONENT
        Devel
)

install(TARGETS GreenIslandPlatform
        EXPORT GreenIslandPlatformTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# CMake package config
set(GreenIslandPlatform_CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/GreenIslandPlatform")
ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/GreenIslandPlatformConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandPlatformConfig.cmake"
                                  INSTALL_DESTINATION ${GreenIslandPlatform_CMAKECONFIG_INSTALL_DIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandPlatformConfig.cmake"
        DESTINATION "${GreenIslandPlatform_CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)
install(EXPORT GreenIslandPlatformTargets
        DESTINATION "${GreenIslandPlatform_CMAKECONFIG_INSTALL_DIR}"
        FILE GreenIslandPlatformTargets.cmake NAMESPACE GreenIsland::)

# qmake pri file
include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME GreenIslandPlatform
                      LIB_NAME GreenIslandPlatform
                      DEPS "core dbus gui"
                      FILENAME_VAR PRI_FILENAME
                      INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}")
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

# pkg-config file
include(ECMGeneratePkgConfigFile)
ecm_generate_pkgconfig_file(BASE_NAME GreenIslandPlatform
                            LIB_NAME GreenIslandPlatform
                            DEPS "Qt5Core Qt5DBus Qt5Gui xkbcommon"
                            FILENAME_VAR PKGCONFIG_FILENAME
                            INCLUDE_INSTALL_DIR "${HAWAII_INCLUDEDIR} ${GREENISLAND_INCLUDEDIR}"
                            INSTALL)
