set(HEADERS
    VIndicator
    VIndicatorPlugin
    VGreenIslandGlobal
)

if(Qt5Compositor_FOUND)
    set(HEADERS
        ${HEADERS}
        VCompositor
        VCompositorPlugin
    )
endif()

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland)

# Create the cmakedirs.h header file
configure_file(cmakedirs.h.in ${CMAKE_CURRENT_BINARY_DIR}/cmakedirs.h)
