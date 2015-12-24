find_package(Libdrm)
set_package_properties(Libdrm PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for DRM/KMS support")
find_package(gbm)
set_package_properties(gbm PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for DRM/KMS support")

include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    eglfskmscursor.cpp
    eglfskmsdevice.cpp
    eglfskmsintegration.cpp
    eglfskmsscreen.cpp
    main.cpp
)

add_library(kms SHARED MODULE ${SOURCES})
target_link_libraries(kms
    GreenIsland::Platform
    Libdrm::Libdrm
    gbm::gbm
)

install(TARGETS kms
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
