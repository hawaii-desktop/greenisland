include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_SOURCE_DIR}/src
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    main.cpp
    application.cpp
    recorder.cpp
)

add_executable(greenisland-screencaster ${SOURCES})
target_link_libraries(greenisland-screencaster GreenIsland::Client)

install(TARGETS greenisland-screencaster DESTINATION ${BIN_INSTALL_DIR})
