include(GenerateExportHeader)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX GREENISLAND
                  VERSION_HEADER "${CMAKE_BINARY_DIR}/headers/greenisland_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfigVersion.cmake"
                  SOVERSION ${PROJECT_SOVERSION})

set(HAWAII_INCLUDEDIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/Hawaii")
set(GREENISLAND_INCLUDEDIR "${HAWAII_INCLUDEDIR}/GreenIsland")

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/GreenIsland")

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/GreenIslandConfig.cmake.in"
                                  "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfig.cmake"
                                  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/GreenIslandConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(FILES ${CMAKE_BINARY_DIR}/headers/greenisland_version.h
        DESTINATION ${HAWAII_INCLUDEDIR} COMPONENT Devel)

include(GenerateHeaders)
include(PrivateHeaders)

add_subdirectory(client)
add_subdirectory(hardwareintegration)
add_subdirectory(platform)
add_subdirectory(server)
add_subdirectory(waylandcompositor)
