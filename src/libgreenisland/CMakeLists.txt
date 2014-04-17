include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
)

add_definitions(
    -DQT_COMPOSITOR_QUICK
)

set(SOURCES
    greenislandcompositor.cpp
    greenislandutilities.cpp
    gldebug.cpp
    surfaceitem.cpp
)

set(HEADERS
    greenislandglobal.h
    greenislandcompositor.h
    greenislandutilities.h
    surfaceitem.h
)

add_library(GreenIsland SHARED ${SOURCES})
qt5_use_modules(GreenIsland Quick Compositor)
set_target_properties(GreenIsland PROPERTIES
    VERSION ${GREENISLAND_VERSION_STRING}
    SOVERSION ${GREENISLAND_VERSION_MAJOR}.${GREENISLAND_VERSION_MINOR}.${GREENISLAND_VERSION_PATCH})

install(TARGETS GreenIsland
        EXPORT GreenIslandTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

target_link_libraries(GreenIsland ${EGL_LIBRARIES})

install(FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland)
install(TARGETS GreenIsland LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR})
