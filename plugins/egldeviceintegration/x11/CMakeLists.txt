if(NOT X11_FOUND)
    message(FATAL_ERROR "X11 is required to build X11 EGL device integration")
endif()
find_package(XCB REQUIRED COMPONENTS XCB)
find_package(X11_XCB REQUIRED)

include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    eglfsx11integration.cpp
    main.cpp
)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    set(GL_TARGET ${OPENGL_LIBRARIES})
else()
    set(GL_TARGET ${OPENGLES_LIBRARIES})
endif()

add_library(x11 SHARED MODULE ${SOURCES})
target_link_libraries(x11
    GreenIsland::Platform
    ${X11_LIBRARIES}
    XCB::XCB
    X11::XCB
    EGL::EGL
    ${GL_TARGET}
)

install(TARGETS x11
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
