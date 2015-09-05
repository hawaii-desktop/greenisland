include_directories(
    ${CMAKE_BINARY_DIR}/src/platform
)

add_executable(tst_logind fakelogind.cpp tst_logind.cpp)
target_link_libraries(tst_logind Qt5::Test GreenIsland::Platform)
add_test(greenisland-test-logind tst_logind)
ecm_mark_as_test(tst_logind)
