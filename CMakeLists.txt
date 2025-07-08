cmake_minimum_required (VERSION 4.0)

set (CMAKE_EXPERIMENTAL_CXX_IMPORT_STD d0edc3af-4c50-42ea-a356-e2862fe7a444)

project (char_db
         VERSION 0.1.0
         LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS ON)
set (CMAKE_CXX_MODULE_STD ON)

option (BUILD_SHARED_LIBS "Build using shared libraries" ON)

include (cmake/ucd_gen.cmake)

add_library (char_db)
target_compile_features (char_db PUBLIC cxx_std_23)
target_sources (
    char_db
    PUBLIC
        FILE_SET
            CXX_MODULES
        FILES
            src/char_db.cc
            src/utils.cc
            src/containers.cc
            src/database.cc
            src/views.cc
    PUBLIC
        FILE_SET
            HEADERS
        BASE_DIRS
            ${UCD_GEN_INCLUDE_DIR}
        FILES
            ${UCD_GEN_OUTPUT_FILES}
)
add_dependencies (char_db char_db_ucd_gen)

include (cmake/install.cmake)

message(STATUS "${PROJECT_NAME} configured. C++ Standard: ${CMAKE_CXX_STANDARD}")