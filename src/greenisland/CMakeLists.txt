include_directories(
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/headers/GreenIsland
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
)

add_definitions(-DQT_COMPOSITOR_QUICK)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY
)

set(SOURCES
    main.cpp
    greenisland.cpp
)

add_executable(greenisland ${SOURCES})
qt5_use_modules(greenisland Gui)
target_link_libraries(greenisland GreenIsland)

# Git revision
include(GetGitRevision)
create_git_head_revision_file(gitsha1.h greenisland)

install(TARGETS greenisland DESTINATION ${CMAKE_INSTALL_BINDIR})
