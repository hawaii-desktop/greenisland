include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

add_executable(tst_libinput tst_libinput.cpp)
target_link_libraries(tst_libinput GreenIsland::Platform)
