include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

add_definitions(-DMESA_EGL_NO_X11_HEADERS)

set(SOURCES
    qeglconvenience.cpp
    qeglpbuffer.cpp
    qeglplatformcontext.cpp
    qeglplatformcursor.cpp
    qeglplatformintegration.cpp
    qeglplatformscreen.cpp
    qeglplatformwindow.cpp
    qxlibeglintegration.cpp
)

add_library(eglconvenience STATIC ${SOURCES})
target_link_libraries(eglconvenience Qt5::Gui)
