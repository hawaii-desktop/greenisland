project(GreenIsland)

cmake_minimum_required(VERSION 2.8.7)

# Enable automoc and include current directory
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set version
set(GREENISLAND_VERSION_MAJOR 0)
set(GREENISLAND_VERSION_MINOR 0)
set(GREENISLAND_VERSION_PATCH 0)
set(GREENISLAND_VERSION_STRING "${GREENISLAND_VERSION_MAJOR}.${GREENISLAND_VERSION_MINOR}.${GREENISLAND_VERSION_PATCH}")

# Installation directories
include(HawaiiInstallDirs)

# Our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include Wayland macros
include(Wayland)

# Add custom uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Set up packaging
set(CPACK_PACKAGE_NAME hawaii-greenisland)
set(CPACK_PACKAGE_VERSION ${GREENISLAND_VERSION_STRING})
set(CPACK_SYSTEM_NAME "generic")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SET_DESTDIR FALSE)
include(CPack)

# Build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -Wall -std=c++0x")

# Assume release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "" FORCE)
endif()

# Disable debug output for release builds
if(CMAKE_BUILD_TYPE MATCHES "^[Rr]elease$")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

# Find Qt5
find_package(Qt5DBus REQUIRED)
if(NOT Qt5DBus_FOUND)
    message(FATAL_ERROR "Qt5DBus module is required!")
endif()
find_package(Qt5Gui REQUIRED)
if(NOT Qt5Gui_FOUND)
    message(FATAL_ERROR "Qt5Gui module is required!")
endif()
find_package(Qt5Widgets REQUIRED)
if(NOT Qt5Widgets_FOUND)
    message(FATAL_ERROR "Qt5Widgets module is required!")
endif()
find_package(Qt5Qml REQUIRED)
if(NOT Qt5Qml_FOUND)
    message(FATAL_ERROR "Qt5Qml module is required!")
endif()
find_package(Qt5Quick REQUIRED)
if(NOT Qt5Quick_FOUND)
    message(FATAL_ERROR "Qt5Quick module is required!")
endif()
find_package(Qt5Compositor REQUIRED)
if(NOT Qt5Compositor_FOUND)
    message(FATAL_ERROR "Qt5Compositor module is required!")
endif()
find_package(Qt5AccountsService REQUIRED)
if(NOT Qt5AccountsService_FOUND)
    message(FATAL_ERROR "Qt5AccountsService module is required!")
endif()

# Find Vibe libraries
find_package(VibeCore REQUIRED)
if(NOT VibeCore_FOUND)
    message(FATAL_ERROR "VibeCore module is required!")
endif()

# Find pkg-config
find_package(PkgConfig "0.22" REQUIRED)

# Find EGL and GLESv2 libraries
pkg_check_modules(EGL egl REQUIRED)
pkg_check_modules(GLESV2 glesv2 REQUIRED)

# Find Wayland libraries
pkg_check_modules(WAYLAND_CLIENT wayland-client REQUIRED)
pkg_check_modules(WAYLAND_SERVER wayland-server REQUIRED)

# Find polkit-qt-1
find_package(PolkitQt-1 REQUIRED)
if(NOT PolkitQt-1_FOUND)
    message(FATAL_ERROR "PolkitQt-1 is required!")
endif()

# Check whether we are building on Rapsberry Pi or not
include(CheckFileExists)
check_file_exists(/opt/vc/include/bcm_host.h BCMHOST)
if(BCMHOST_FOUND)
    include_directories(/opt/vc/include)
endif()

# Subdirectories
add_subdirectory(data)
add_subdirectory(headers)
add_subdirectory(src)
