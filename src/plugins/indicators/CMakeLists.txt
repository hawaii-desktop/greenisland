include_directories(
    ${Qt5Quick_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
)

add_definitions(
    ${Qt5Quick_DEFINITIONS}
    -DQT_PLUGIN
)

set(SOURCES
    main.cpp
    datetime/datetimeindicator.cpp
    datetime/datetimeview.cpp
    power/powerindicator.cpp
    power/powerview.cpp
)

qt5_add_resources(RESOURCES indicators.qrc)
add_library(greenislandindicators SHARED ${SOURCES} ${RESOURCES})

target_link_libraries(greenislandindicators
    ${Qt5Quick_LIBRARIES}
    GreenIsland
)

install(TARGETS greenislandindicators LIBRARY DESTINATION ${CMAKE_INSTALL_PLUGINSDIR}/greenisland/indicators)
