include_directories(
    ${CMAKE_BINARY_DIR}/headers
    ${CMAKE_BINARY_DIR}/headers/GreenIsland
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    eglfsbrcmintegration.cpp
    main.cpp
)

add_library(brcm SHARED MODULE ${SOURCES})
target_link_libraries(brcm
    GreenIsland::Platform
    BcmHost::BcmHost
    BcmHost::EGL
    BcmHost::GLESv2
)

install(TARGETS brcm
        DESTINATION ${PLUGIN_INSTALL_DIR}/greenisland/egldeviceintegration)
