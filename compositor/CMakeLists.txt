include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

set(SOURCES
    main.cpp
)

add_executable(greenisland ${SOURCES})
target_link_libraries(greenisland
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Qml
        GreenIsland::Server
)

if(systemd_FOUND)
    target_link_libraries(greenisland PRIVATE ${systemd_LIBRARIES})
endif()

install(TARGETS greenisland DESTINATION ${BIN_INSTALL_DIR})
