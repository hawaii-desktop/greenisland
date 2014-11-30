add_definitions(-DTRANSLATION_DOMAIN=\"kpackage_package_greenisland\")

set(SOURCES
    compositorpackage.cpp
)

add_library(kpackage_packagestructure_greenisland MODULE ${SOURCES})
target_link_libraries(kpackage_packagestructure_greenisland
    KF5::I18n
    KF5::Package
)

kcoreaddons_desktop_to_json(kpackage_packagestructure_greenisland kpackage-packagestructure-greenisland.desktop)

install(TARGETS kpackage_packagestructure_greenisland
        DESTINATION ${PLUGIN_INSTALL_DIR}/kpackage/packagestructure)
