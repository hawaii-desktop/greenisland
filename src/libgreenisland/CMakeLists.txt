include_directories(
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Compositor_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
)

add_definitions(${Qt5Quick_DEFINITIONS})

set(SOURCES
    vindicator.cpp
    vindicatorplugin.cpp
    vshell.cpp
    vshellplugin.cpp
)

set(HEADERS
    vindicator.h
    vindicatorplugin.h
    vshell.h
    vshellplugin.h
)

add_library(GreenIsland SHARED ${SOURCES})
target_link_libraries(GreenIsland
    ${Qt5Quick_LIBRARIES}
    ${Qt5Compositor_LIBRARIES}
)
set_target_properties(GreenIsland PROPERTIES
    VERSION ${GREENISLAND_VERSION_STRING}
    SOVERSION ${GREENISLAND_VERSION_MAJOR}.${GREENISLAND_VERSION_MINOR}.${GREENISLAND_VERSION_PATCH})

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland)
install(TARGETS GreenIsland LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
