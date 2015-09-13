if (NOT TARGET GreenIsland::greenisland-wayland-scanner)
    add_executable(GreenIsland::greenisland-wayland-scanner IMPORTED)

    set(imported_location "${PACKAGE_PREFIX_DIR}/bin/greenisland-wayland-scanner")

    set_target_properties(GreenIsland::greenisland-wayland-scanner PROPERTIES
        IMPORTED_LOCATION ${imported_location}
    )
endif()

set(GreenIsland_WAYLAND_SCANNER_EXECUTABLE GreenIsland::greenisland-wayland-scanner)
