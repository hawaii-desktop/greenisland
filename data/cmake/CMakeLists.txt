include(CMakePackageConfigHelpers)

set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/GreenIsland")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/GreenIslandConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS CMAKE_INSTALL_PREFIX CMAKE_INSTALL_INCLUDEDIR)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfigVersion.cmake"
                                 VERSION ${GREENISLAND_VERSION_STRING}
                                 COMPATIBILITY AnyNewerVersion)

# Install CMake package
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)
install(EXPORT GreenIslandTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE GreenIslandTargets.cmake
        NAMESPACE Hawaii::)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/greenisland_version.h
#        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#        COMPONENT Devel)
