include(ECMMarkAsTest)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test QuickTest)

add_subdirectory(auto)
add_subdirectory(manual)
