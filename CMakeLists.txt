cmake_minimum_required(VERSION 2.6)

project(openbrain)

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)

include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} )

set(openbrain_SRCS openbrain.cpp aimlparser.cpp)

kde4_add_ui_files(openbrain_SRCS openbrainconfig.ui )
kde4_add_plugin(plasma_applet_openbrain ${openbrain_SRCS})

target_link_libraries(plasma_applet_openbrain ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTWEBKIT_LIBRARY} ${QT_QTXML_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS plasma_applet_openbrain DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-openbrain.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/openbrain_configuration DESTINATION ${DATA_INSTALL_DIR}/openbrain PATTERN scripts/* PERMISSIONS WORLD_EXECUTE WORLD_WRITE WORLD_READ OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ)
