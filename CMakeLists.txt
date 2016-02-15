project(GreenIsland)

cmake_minimum_required(VERSION 2.8.12)

# Silence CMake warnings
if(POLICY CMP0053)
    cmake_policy(SET CMP0053 NEW)
endif()
if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

# Set version
set(PROJECT_VERSION "0.7.90")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_SOVERSION 0)

# Set up packaging
set(CPACK_PACKAGE_NAME "greenisland")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SET_DESTDIR FALSE)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git;/.tx/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# Options
option(ENABLE_XWAYLAND "Enables XWayland support" ON)
option(ENABLE_EGLDEVICEINTEGRATION_KMS "Enables DRM/KMS device integration" ON)
option(ENABLE_EGLDEVICEINTEGRATION_X11 "Enables X11 device integration" ON)
option(ENABLE_EGLDEVICEINTEGRATION_BRCM "Enables Broadcom device integration" ON)
option(ENABLE_EGLDEVICEINTEGRATION_MALI "Enables Mali device integration" OFF)
option(ENABLE_EGLDEVICEINTEGRATION_VIV "Enables Vivante device integration" OFF)
option(ENABLE_ONLY_EGLDEVICEINTEGRATION "Build and install only device integration plugins" OFF)

# Enable QML debugger for debug or release with debug info builds
if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb]")
    add_definitions(-DQT_QML_DEBUG)
endif()

# ECM setup
find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_SOURCE_DIR}/cmake")

# Macros
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

# Require at least gcc 4.8
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8")
        message(SEND_ERROR "Version ${CMAKE_CXX_COMPILER_VERSION} of the ${CMAKE_CXX_COMPILER_ID} C++ compiler is not supported. Please use version 4.8 or later.")
    endif()
endif()

# Adjusting CMAKE_C_FLAGS to get Wayland protocols to compile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")

# Add custom uninstall target
if ("${ECM_VERSION}" VERSION_LESS "1.7.0")
    # ECM 1.7.0 provides this target, so we can't roll our own
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)
    add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

# Find Qt5
set(REQUIRED_QT_VERSION 5.5.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core DBus Gui Qml Quick)

# OpenGL
find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES
    DESCRIPTION "The OpenGL libraries"
    URL "http://www.opengl.org"
    TYPE OPTIONAL)

# OpenGL ES
find_package(OpenGLES)
set_package_properties(OpenGLES PROPERTIES
    DESCRIPTION "The OpenGLES libraries"
    URL "http://www.khronos.org/opengles"
    TYPE OPTIONAL)

# EGL
find_package(EGL REQUIRED)

# GL requirements
if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    if(NOT OPENGL_FOUND)
        message(FATAL_ERROR "OpenGL is required")
    endif()
else()
    if(NOT OPENGLES_FOUND)
        message(FATAL_ERROR "OpenGL ES is required")
    endif()
endif()

# brcm
find_package(BcmHost)

# X11
find_package(X11)
set_package_properties(X11 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for X11 EGL device integration and XWayland")
add_feature_info("Xcomposite" X11_Xcomposite_FOUND "Required for xcomposite-egl and xcomposite-glx")

# Wayland
if(NOT ENABLE_ONLY_EGLDEVICEINTEGRATION)
    set(REQUIRED_WAYLAND_VERSION 1.6.0)
    find_package(Wayland ${REQUIRED_WAYLAND_VERSION} COMPONENTS Client Server Cursor Egl)
    set_package_properties(Wayland PROPERTIES
        TYPE REQUIRED
        PURPOSE "Required to build Green Island")
    add_feature_info("Wayland-Client" Wayland_Client_FOUND "Required for protocols")
    add_feature_info("Wayland-Server" Wayland_Server_FOUND "Required for protocols")
    add_feature_info("Wayland-Cursor" Wayland_Cursor_FOUND "Required for changing cursors")
    add_feature_info("Wayland-Egl" Wayland_Egl_FOUND "Required for hardware integration")
endif()

# systemd
pkg_check_modules(systemd libsystemd-daemon)
if(systemd_FOUND)
    set(HAVE_SYSTEMD 1)
else()
    # libsystem-daemon was merged to libsystem on systemd 208->209
    pkg_check_modules(systemd libsystemd)
    if(systemd_FOUND)
        set(HAVE_SYSTEMD 1)
    endif()
endif()
add_feature_info("systemd" systemd_FOUND "Required for systemd integration")

# xkbcommon
pkg_check_modules(xkbcommon xkbcommon REQUIRED)
add_feature_info("xkbcommon" xkbcommon_FOUND "Required for keymap support")

# Subdirectories
if(ENABLE_ONLY_EGLDEVICEINTEGRATION)
    add_subdirectory(plugins)
else()
    add_subdirectory(compositor)
    add_subdirectory(data)
    add_subdirectory(declarative)
    add_subdirectory(headers)
    add_subdirectory(launcher)
    add_subdirectory(plugins)
    add_subdirectory(qpa)
    add_subdirectory(shells)
    add_subdirectory(src)
    add_subdirectory(tests)
    add_subdirectory(tools)
endif()

# Display featute summary
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
