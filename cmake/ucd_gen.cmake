function(check_python_library LIBRARY_NAME)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import ${LIBRARY_NAME};  print(${LIBRARY_NAME}.__version__ if hasattr(${LIBRARY_NAME}, '__version__') else 'oh')"
        OUTPUT_VARIABLE PROC_RESULT OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if (NOT ${PROC_RESULT} EQUAL oh)
        set(PYTHON_LIBRARY_${LIBRARY_NAME}_VERSION ${PROC_RESULT} PARENT_SCOPE)
    endif()
endfunction()

set(UCD_GEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/include)
set(UCD_GEN_OUTPUT_DIR ${UCD_GEN_INCLUDE_DIR}/generated)
set(UCD_GEN_INTERMEDIATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ucd)
set(UCD_GEN_OUTPUT_FILES
    ${UCD_GEN_OUTPUT_DIR}/bmp_ranges.inc
    ${UCD_GEN_OUTPUT_DIR}/non_bmp_ranges.inc
    ${UCD_GEN_OUTPUT_DIR}/utf8_ranges_1.inc
    ${UCD_GEN_OUTPUT_DIR}/utf8_ranges_2.inc
    ${UCD_GEN_OUTPUT_DIR}/utf8_ranges_3.inc
    ${UCD_GEN_OUTPUT_DIR}/utf8_ranges_4.inc
    ${UCD_GEN_OUTPUT_DIR}/just_ranges.inc
)

add_custom_command(
    OUTPUT
        ${UCD_GEN_OUTPUT_FILES}
    COMMAND
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/ucd_gen.py
        ${UCD_GEN_INTERMEDIATE_DIR}
        ${UCD_GEN_OUTPUT_DIR}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/ucd_gen.py
    COMMENT
        "Generating Unicode Character Database (UCD) C++ files"
    VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)