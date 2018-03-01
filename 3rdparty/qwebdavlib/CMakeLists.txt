remove_definitions( -DQT_NO_CAST_FROM_ASCII )
remove_definitions( -DQT_NO_CAST_TO_ASCII )
remove_definitions( -DQT_NO_URL_CAST_FROM_STRING )
set(CMAKE_AUTOMOC TRUE)

add_library(kdav2_webdavlib
    qwebdav.cpp
)

target_link_libraries(kdav2_webdavlib
PUBLIC
    Qt5::Xml
    Qt5::Network
)

generate_export_header(kdav2_webdavlib BASE_NAME qwebdavshared)

install(TARGETS
    kdav2_webdavlib
    EXPORT KPimKDAV2Targets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)
