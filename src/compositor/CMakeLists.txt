include_directories(
    ${CMAKE_BINARY_DIR}/headers
)

set(SOURCES
    main.cpp
)

#qt5_add_resources(RESOURCES greenisland.qrc)

add_executable(greenisland ${SOURCES} ${RESOURCES})
target_link_libraries(greenisland
    GreenIsland::GreenIsland
)

install(TARGETS greenisland DESTINATION ${BIN_INSTALL_DIR})
