include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
)

set(SOURCES
    vindicator.cpp
    vindicatorplugin.cpp
)

set(HEADERS
    vindicator.h
    vindicatorplugin.h
    vgreenislandglobal.h
)

if(Qt5Compositor_FOUND)
    set(SOURCES
        ${SOURCES}
        vcompositor.cpp
        vcompositorplugin.cpp
    )
    set(HEADERS
        ${HEADERS}
        vcompositor.h
        vcompositorplugin.h
    )
endif()

add_library(GreenIsland SHARED ${SOURCES})
if(Qt5Compositor_FOUND)
    set(GreenIsland_MODULES Quick Compositor)
else()
    set(GreenIsland_MODULES Quick)
endif()
qt5_use_modules(GreenIsland ${GreenIsland_MODULES})
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
