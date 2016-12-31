include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/../headers"
)

set(SOURCES
    main.cpp
    compositorlauncher.cpp
    compositorprocess.cpp
)

add_executable(greenisland-launcher ${SOURCES})
target_link_libraries(greenisland-launcher Qt5::Core)

install(TARGETS greenisland-launcher DESTINATION ${BIN_INSTALL_DIR})
