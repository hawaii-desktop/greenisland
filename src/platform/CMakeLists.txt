pkg_check_modules(gio gio-2.0 REQUIRED)
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

find_package(Threads)
set_package_properties(Threads PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required by the vt handler code")

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${UDEV_INCLUDE_DIR}
    ${gio_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/headers
)

add_definitions(
    -DMESA_EGL_NO_X11_HEADERS
    -DQ_FONTCONFIGDATABASE
)

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
)

qt5_add_resources(SOURCES deviceintegration/cursor.qrc)

add_library(GreenIslandPlatform SHARED ${SOURCES})
generate_export_header(GreenIslandPlatform EXPORT_FILE_NAME greenislandplatform/greenisland_platform_export.h)
add_library(GreenIsland::Platform ALIAS GreenIslandPlatform)
target_link_libraries(GreenIslandPlatform
  PUBLIC
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    EGL::EGL
  PRIVATE
    ${UDEV_LIBS}
    Libinput::Libinput
    ${gio_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    Qt5PlatformSupport::Qt5PlatformSupport
    Threads::Threads
)

if(xkbcommon_FOUND)
    include_directories(${xkbcommon_INCLUDE_DIRS})
    add_definitions(-DHAVE_XKBCOMMON)
    target_link_libraries(GreenIslandPlatform PRIVATE ${xkbcommon_LIBRARIES})
endif()

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    target_link_libraries(GreenIslandPlatform PRIVATE ${OPENGL_LIBRARIES})
else()
    target_link_libraries(GreenIslandPlatform PRIVATE ${OPENGLES_LIBRARIES})
endif()

ecm_generate_headers(deviceintegration_HEADERS
  HEADER_NAMES
    EGLDeviceIntegration
    EglFSContext
    EglFSCursor
    EglFSIntegration
    EglFSOffscreenWindow
    EglFSScreen
    EglFSWindow
  PREFIX
    GreenIslandPlatform
  REQUIRED_HEADERS
    GreenIslandPlatform_HEADERS
  RELATIVE
    deviceintegration
)

ecm_generate_headers(udev_HEADERS
  HEADER_NAMES
    Udev
    UdevDevice
    UdevEnumerate
    UdevMonitor
  PREFIX
    GreenIslandPlatform
  REQUIRED_HEADERS
    GreenIslandPlatform_HEADERS
  RELATIVE
    udev
)

ecm_generate_headers(logind_HEADERS
  HEADER_NAMES
    Logind
    VtHandler
  PREFIX
    GreenIslandPlatform
  REQUIRED_HEADERS
    GreenIslandPlatform_HEADERS
  RELATIVE
    logind
)

ecm_generate_headers(libinput_HEADERS
  HEADER_NAMES
    LibInputHandler
  PREFIX
    GreenIslandPlatform
  REQUIRED_HEADERS
    GreenIslandPlatform_HEADERS
  RELATIVE
    libinput
)
