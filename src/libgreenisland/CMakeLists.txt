include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
)

set(SOURCES
    vcompositor.cpp
    vcompositorplugin.cpp
    vindicator.cpp
    vindicatorplugin.cpp
)

set(HEADERS
    vcompositor.h
    vcompositorplugin.h
    vindicator.h
    vindicatorplugin.h
)

add_library(GreenIsland SHARED ${SOURCES})
qt5_use_modules(GreenIsland Quick Compositor)
set_target_properties(GreenIsland PROPERTIES
    VERSION ${GREENISLAND_VERSION_STRING}
    SOVERSION ${GREENISLAND_VERSION_MAJOR}.${GREENISLAND_VERSION_MINOR}.${GREENISLAND_VERSION_PATCH})

install(FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland)
install(TARGETS GreenIsland LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR})
