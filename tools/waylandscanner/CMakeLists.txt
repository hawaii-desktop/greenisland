include_directories(
    ${CMAKE_BINARY_DIR}/headers
)

add_executable(greenisland-wayland-scanner waylandscanner.cpp)
target_link_libraries(greenisland-wayland-scanner Qt5::Core)
add_executable(GreenIsland::greenisland-wayland-scanner ALIAS greenisland-wayland-scanner)

install(TARGETS greenisland-wayland-scanner
        DESTINATION ${BIN_INSTALL_DIR}
        COMPONENT Devel)
