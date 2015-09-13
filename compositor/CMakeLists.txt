include_directories(
    ${CMAKE_BINARY_DIR}/headers
)

set(SOURCES
    main.cpp
)

add_executable(greenisland ${SOURCES})
target_link_libraries(greenisland
    PUBLIC
        GreenIsland::GreenIsland
)

if(systemd_FOUND)
    target_link_libraries(greenisland PRIVATE ${systemd_LIBRARIES})
endif()

install(TARGETS greenisland DESTINATION ${BIN_INSTALL_DIR})
