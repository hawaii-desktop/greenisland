include_directories(
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Compositor_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
)

if(BCMHOST)
    include_directories(/opt/vc/include)
endif()

add_definitions(
    ${Qt5Quick_DEFINITIONS}
    ${Qt5Compositor_DEFINITIONS}
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
target_link_libraries(GreenIsland
    ${Qt5Quick_LIBRARIES}
    ${Qt5Compositor_LIBRARIES}
)
set_target_properties(GreenIsland PROPERTIES
    VERSION ${GREENISLAND_VERSION_STRING}
    SOVERSION ${GREENISLAND_VERSION_MAJOR}.${GREENISLAND_VERSION_MINOR}.${GREENISLAND_VERSION_PATCH})

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland)
install(TARGETS GreenIsland LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
