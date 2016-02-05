set(FILES
    four-960x540.json
    fullhd-hidpi.json
    one-1024x768.json
    one-1920x1080.json
    one-960x540.json
    phone-1080x1920.json
    phone-720x1280.json
    rotation-inverted.json
    rotation-left.json
    rotation-right.json
    tablet-2048x1536.json
    two-1024x768.json
    two-rotated.json
    two-uneven.json
)

install(FILES ${FILES} DESTINATION ${DATA_INSTALL_DIR}/greenisland/screen-data)
