include_directories(
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/libgreenisland
    ${CMAKE_BINARY_DIR}/src/libgreenisland
)

set(SOURCES
    waylandeglclientbufferintegration.cpp
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    set(GL_TARGET ${OPENGL_LIBRARIES})
else()
    set(GL_TARGET ${OPENGLES_LIBRARIES})
endif()

add_library(wayland-egl SHARED ${SOURCES})
target_link_libraries(wayland-egl
  PUBLIC
    GreenIsland::GreenIsland
    Wayland::Server
    Wayland::Egl
  PRIVATE
    EGL::EGL
    ${GL_TARGET}
)

install(TARGETS wayland-egl
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/hardware-integration/client)
