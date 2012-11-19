set(HEADERS
    VCompositor
    VCompositorPlugin
    VIndicator
    VIndicatorPlugin
    VGreenIslandGlobal
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland)

# Create the cmakedirs.h header file
configure_file(cmakedirs.h.in ${CMAKE_CURRENT_BINARY_DIR}/cmakedirs.h)
