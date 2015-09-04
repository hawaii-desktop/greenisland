include(GenerateExportHeader)
include(ECMGenerateHeaders)

include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${UDEV_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/headers
)

set(SOURCES
    logging.cpp
    udev/udev.cpp
    udev/udevdevice.cpp
    udev/udevenumerate.cpp
    udev/udevmonitor.cpp
    logind/logind.cpp
    libinput/libinputgesture.cpp
    libinput/libinputhandler.cpp
    libinput/libinputmanager_p.cpp
    libinput/libinputkeyboard.cpp
    libinput/libinputpointer.cpp
    libinput/libinputtouch.cpp
)

add_library(GreenIslandPlatform SHARED ${SOURCES})
generate_export_header(GreenIslandPlatform EXPORT_FILE_NAME greenislandplatform/greenisland_platform_export.h)
add_library(GreenIsland::Platform ALIAS GreenIslandPlatform)
target_link_libraries(GreenIslandPlatform
  PUBLIC
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5PlatformSupport::Qt5PlatformSupport
  PRIVATE
    ${UDEV_LIBS}
    Libinput::Libinput
)

if(xkbcommon_FOUND)
    include_directories(${xkbcommon_INCLUDE_DIRS})
    add_definitions(-DHAVE_XKBCOMMON)
    target_link_libraries(GreenIslandPlatform PRIVATE ${xkbcommon_LIBRARIES})
endif()

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
