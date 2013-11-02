set(HEADERS
    GreenIslandGlobal
)

if(Qt5Compositor_FOUND)
    set(HEADERS
        ${HEADERS}
        Compositor
    )
endif()

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GreenIsland)

# Create the cmakedirs.h header file
configure_file(cmakedirs.h.in ${CMAKE_CURRENT_BINARY_DIR}/cmakedirs.h)
