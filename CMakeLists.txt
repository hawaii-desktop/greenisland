project(GreenIsland)

cmake_minimum_required(VERSION 2.8.12)

# Set version
set(PROJECT_VERSION "0.5.90")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_SOVERSION 0)

# Add custom uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Set up packaging
set(CPACK_PACKAGE_NAME "greenisland")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SET_DESTDIR FALSE)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git;/.tx/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# Enable QML debugger for debug or release with debug info builds
if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb]")
    add_definitions(-DQT_QML_DEBUG)
endif()

# ECM setup
find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_SOURCE_DIR}/cmake")

# Options
option(ENABLE_OPENGL "Enable OpenGL support" OFF)

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

    # Adjusting CMAKE_C_FLAGS to get wayland protocols to compile
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")
endif()

# Find Qt5
set(REQUIRED_QT_VERSION 5.3.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus Gui Qml Quick Compositor)

# Find KF5
set(REQUIRED_KF5_VERSION 5.0.91)
find_package(KF5 ${REQUIRED_KF5_VERSION} REQUIRED COMPONENTS I18n Screen Service Package)

# GL requirements
if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    find_package(OpenGL)
    set_package_properties(OpenGL PROPERTIES
        DESCRIPTION "The OpenGL libraries"
        URL "http://www.opengl.org"
        TYPE REQUIRED)
else()
    find_package(OpenGLES)
    set_package_properties(OpenGLES PROPERTIES
        DESCRIPTION "The OpenGLES libraries"
        URL "http://www.khronos.org/opengles"
        TYPE REQUIRED)
endif()

find_package(EGL)
set_package_properties(EGL PROPERTIES
    TYPE RUNTIME
    PURPOSE "Required to build Green Island with EGL support")

# Wayland
set(REQUIRED_WAYLAND_VERSION 1.5)
find_package(Wayland ${REQUIRED_WAYLAND_VERSION} COMPONENTS Client Server Egl)
set_package_properties(Wayland PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required to build Green Island")
add_feature_info("Wayland-Client" Wayland_Client_FOUND "Required for protocols")
add_feature_info("Wayland-Server" Wayland_Server_FOUND "Required for protocols")
add_feature_info("Wayland-Egl" Wayland_Egl_FOUND "Required for the compositor")

# Wayland and QtWayland scanner
find_package(QtWaylandScanner REQUIRED)

# systemd
pkg_check_modules(systemd libsystemd-daemon)
if(systemd_FOUND)
    set(HAVE_SYSTEMD 1)
endif()
add_feature_info("systemd" systemd_FOUND "Required for systemd integration")

# Subdirectories
add_subdirectory(headers)
add_subdirectory(src)

# Display featute summary
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
