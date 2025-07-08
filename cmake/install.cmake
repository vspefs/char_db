include (GNUInstallDirs)

install (
    TARGETS
        char_db
    EXPORT
        char_db_exported_targets

    ARCHIVE DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET CXX_MODULES DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}/char_db
)

install (
    EXPORT
        char_db_exported_targets
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/char_db
    NAMESPACE
        char_db::
    CXX_MODULES_DIRECTORY
        cxx-modules
)

include (CMakePackageConfigHelpers)

write_basic_package_version_file (
    ${CMAKE_CURRENT_BINARY_DIR}/char_db-config-version.cmake
    VERSION
        ${PROJECT_VERSION}
    COMPATIBILITY
        ExactVersion
)

configure_package_config_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/char_db-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/char_db-config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/char_db
    PATH_VARS
        CMAKE_INSTALL_LIBDIR
)

install (
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/char_db-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/char_db-config-version.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/char_db
)