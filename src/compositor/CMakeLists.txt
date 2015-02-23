include_directories(
    ${CMAKE_BINARY_DIR}/headers
)

set(SOURCES
    main.cpp
)

add_executable(greenisland ${SOURCES})
target_link_libraries(greenisland
    GreenIsland::GreenIsland
)

install(TARGETS greenisland DESTINATION ${BIN_INSTALL_DIR})
