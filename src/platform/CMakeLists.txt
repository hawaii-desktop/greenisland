include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${UDEV_INCLUDE_DIR}
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

add_library(GreenIslandPlatform STATIC ${SOURCES})
target_link_libraries(GreenIslandPlatform
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5PlatformSupport::Qt5PlatformSupport
    ${UDEV_LIBS}
    Libinput::Libinput
)

if(xkbcommon_FOUND)
    include_directories(${xkbcommon_INCLUDE_DIRS})
    add_definitions(-DHAVE_XKBCOMMON)
    target_link_libraries(GreenIslandPlatform ${xkbcommon_LIBRARIES})
endif()
