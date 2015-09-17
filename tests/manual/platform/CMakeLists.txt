include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/src/platform
)

add_executable(tst_libinput tst_libinput.cpp)
target_link_libraries(tst_libinput GreenIsland::Platform)
