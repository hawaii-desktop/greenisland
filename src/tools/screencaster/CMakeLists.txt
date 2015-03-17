include_directories(
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

set(SOURCES
    main.cpp
    screencaster.cpp
)

ecm_add_qtwayland_client_protocol(SOURCES
    PROTOCOL ${CMAKE_SOURCE_DIR}/data/protocols/greenisland-recorder.xml
    BASENAME greenisland-recorder
    PREFIX greenisland_
)

add_executable(greenisland-screencaster ${SOURCES})
target_link_libraries(greenisland-screencaster Qt5::Gui Wayland::Client)

install(TARGETS greenisland-screencaster DESTINATION ${BIN_INSTALL_DIR})
