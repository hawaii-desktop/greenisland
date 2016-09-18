include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers"
    "${CMAKE_CURRENT_BINARY_DIR}/../../../headers/GreenIsland"
)

add_executable(tst_logind fakelogind.cpp tst_logind.cpp)
target_link_libraries(tst_logind Qt5::Test GreenIsland::Platform)
add_test(greenisland-test-logind tst_logind)
ecm_mark_as_test(tst_logind)

add_executable(tst_udev tst_udev.cpp)
target_link_libraries(tst_udev Qt5::Test GreenIsland::Platform)
add_test(greenisland-test-udev tst_udev)
ecm_mark_as_test(tst_udev)
