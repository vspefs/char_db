cmake_minimum_required(VERSION 4.0)

project(char_db VERSION 0.1.0 LANGUAGES CXX)

include(cmake/ucd_gen.cmake)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
message(STATUS "Python3 interpreter found: ${Python3_EXECUTABLE}")
message(STATUS "Python3 version: ${Python3_VERSION}")

check_python_library(requests)
if(NOT DEFINED PYTHON_LIBRARY_requests_VERSION)
    message(FATAL_ERROR "Python library 'requests' not found. Please install it for Python3 interpreter: ${Python3_EXECUTABLE}")
else()
    message(STATUS "Python library 'requests' found, version: ${PYTHON_LIBRARY_requests_VERSION}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(char_db INTERFACE)
target_include_directories(char_db INTERFACE
        $<BUILD_INTERFACE:${UCD_GEN_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(char_db INTERFACE cxx_std_23)
add_custom_target(char_db_ucd_gen
    DEPENDS ${UCD_GEN_OUTPUT_FILES}
    COMMENT "Generating Unicode Character Database (UCD) C++ files"
)
add_dependencies(char_db char_db_ucd_gen)

if(BUILD_EXAMPLE)
    add_executable(example example.cc)
    target_link_libraries(example PRIVATE char_db)
endif()

include(GNUInstallDirs)
install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hh"
)
install(
        DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.tcc"
)
install(
        DIRECTORY ${UCD_GEN_INCLUDE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/char_db
        FILES_MATCHING PATTERN "*.inc"
)
install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
        EXPORT ${PROJECT_NAME}_targets
        FILE ${PROJECT_NAME}_targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)
install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

message(STATUS "${PROJECT_NAME} configured. C++ Standard: ${CMAKE_CXX_STANDARD}")