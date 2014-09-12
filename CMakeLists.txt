project(GreenIsland)

cmake_minimum_required(VERSION 2.8.12)

option(USE_OPENGL "Enable OpenGL code" OFF)

# Silence CMake 3.0 warnings
if(POLICY CMP0043)
    cmake_policy(SET CMP0043 OLD)
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Set version
set(GREENISLAND_VERSION_MAJOR 0)
set(GREENISLAND_VERSION_MINOR 2)
set(GREENISLAND_VERSION_PATCH 90)
set(GREENISLAND_VERSION_STRING "${GREENISLAND_VERSION_MAJOR}.${GREENISLAND_VERSION_MINOR}.${GREENISLAND_VERSION_PATCH}")

# Add custom uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Set up packaging
set(CPACK_PACKAGE_NAME "greenisland")
set(CPACK_PACKAGE_VERSION "${GREENISLAND_VERSION_STRING}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SET_DESTDIR FALSE)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git;/.tx/;~$;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

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

# Enable QML debugger for debug or release with debug info builds
if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb]")
    add_definitions(-DQT_QML_DEBUG)
endif()

# ECM setup
find_package(ECM 0.0.10 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# KDE macros
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

# Macro to print feature summary
include(FeatureSummary)

# Require at least gcc 4.8
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.8")
        message(SEND_ERROR "Version ${CMAKE_CXX_COMPILER_VERSION} of the ${CMAKE_CXX_COMPILER_ID} C++ compiler is not supported. Please use version 4.8 or later.")
    endif()

    # Adjusting CMAKE_C_FLAGS to get wayland protocols to compile
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")
endif()

# Minimum version requirements
set(QT_MIN_VERSION "5.3.0")
set(KF5_MIN_VERSION "5.0.0")

# Find Qt5
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS DBus Gui Qml Quick Compositor)

# Find KF5
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Screen)

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
find_package(Wayland 1.5 COMPONENTS Client Server Egl)
set_package_properties(Wayland PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required to build Green Island")
add_feature_info("Wayland-Client" Wayland_Client_FOUND "Required for protocols")
add_feature_info("Wayland-Server" Wayland_Server_FOUND "Required for protocols")
add_feature_info("Wayland-Egl" Wayland_Egl_FOUND "Required for the compositor")

# Include Wayland macros
include(ECMWayland)

# Subdirectories
add_subdirectory(headers)
add_subdirectory(src)

# Display featute summary
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
