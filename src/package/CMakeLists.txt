add_definitions(-DTRANSLATION_DOMAIN=\"plasma_package_greenisland_compositor\")

set(SOURCES
    compositorpackage.cpp
)

add_library(plasma_packagestructure_greenisland_compositor MODULE ${SOURCES})
target_link_libraries(plasma_packagestructure_greenisland_compositor
    KF5::I18n
    KF5::Package
)

kservice_desktop_to_json(plasma_packagestructure_greenisland_compositor plasma-packagestructure-greenisland-compositor.desktop)

install(TARGETS plasma_packagestructure_greenisland_compositor
        DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/packagestructure)
