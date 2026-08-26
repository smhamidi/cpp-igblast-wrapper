cmake_minimum_required(VERSION 3.16)
project(cpp_igblast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(XXD_EXECUTABLE xxd)

if(XXD_EXECUTABLE)
    message(STATUS "Using xxd for binary embedding: ${XXD_EXECUTABLE}")
else()
    message(STATUS "xxd not found; using Perl hex embed fallback")
endif()

set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated_embeds")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

set(EMBED_SPEC "")

list(APPEND EMBED_SPEC "bin/igblastn|${CMAKE_SOURCE_DIR}/data/ncbi-igblast-1.22.0/bin/igblastn|1")

file(GLOB COMPILED_DB_FILES LIST_DIRECTORIES false "${CMAKE_SOURCE_DIR}/data/compiled_db/*")
list(SORT COMPILED_DB_FILES)
foreach(DB_FILE IN LISTS COMPILED_DB_FILES)
    get_filename_component(DB_NAME "${DB_FILE}" NAME)
    list(APPEND EMBED_SPEC "database/${DB_NAME}|${DB_FILE}|0")
endforeach()

file(GLOB INTERNAL_HUMAN_FILES LIST_DIRECTORIES false
     "${CMAKE_SOURCE_DIR}/data/ncbi-igblast-1.22.0/internal_data/human/*")
list(SORT INTERNAL_HUMAN_FILES)
foreach(INTERNAL_FILE IN LISTS INTERNAL_HUMAN_FILES)
    get_filename_component(INTERNAL_NAME "${INTERNAL_FILE}" NAME)
    list(APPEND EMBED_SPEC "internal_data/human/${INTERNAL_NAME}|${INTERNAL_FILE}|0")
endforeach()

list(APPEND EMBED_SPEC
     "optional_file/human_gl.aux|${CMAKE_SOURCE_DIR}/data/ncbi-igblast-1.22.0/optional_file/human_gl.aux|0")

set(EMBED_HEADERS "")
set(MANIFEST_INCLUDES "")
set(MANIFEST_ENTRIES "")

foreach(SPEC IN LISTS EMBED_SPEC)
    string(REPLACE "|" ";" PARTS "${SPEC}")
    list(GET PARTS 0 REL_PATH)
    list(GET PARTS 1 ABS_PATH)
    list(GET PARTS 2 IS_EXEC)

    if(NOT EXISTS "${ABS_PATH}")
        message(FATAL_ERROR "Embed source not found: ${ABS_PATH}")
    endif()

    string(REPLACE "/" "_" SAFE_NAME "${REL_PATH}")
    string(REPLACE "." "_" SAFE_NAME "${SAFE_NAME}")
    string(REPLACE "-" "_" SAFE_NAME "${SAFE_NAME}")

    set(SYMBOL "embed_${SAFE_NAME}")
    set(HEADER "${GENERATED_DIR}/${SYMBOL}.h")

    if(XXD_EXECUTABLE)
        add_custom_command(
            OUTPUT "${HEADER}"
            COMMAND "${XXD_EXECUTABLE}" -i -n "${SYMBOL}" "${ABS_PATH}" > "${HEADER}"
            DEPENDS "${ABS_PATH}"
            COMMENT "Embedding ${REL_PATH}"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT "${HEADER}"
            COMMAND perl "${CMAKE_SOURCE_DIR}/cmake/hexembed.pl"
                    "${ABS_PATH}" "${HEADER}" "${SYMBOL}"
            DEPENDS "${ABS_PATH}" "${CMAKE_SOURCE_DIR}/cmake/hexembed.pl"
            COMMENT "Embedding ${REL_PATH}"
            VERBATIM
        )
    endif()

    list(APPEND EMBED_HEADERS "${HEADER}")
    get_filename_component(HEADER_NAME "${HEADER}" NAME)
    list(APPEND MANIFEST_INCLUDES "#include \"${HEADER_NAME}\"")

    if(IS_EXEC STREQUAL "1")
        set(EXEC_FLAG "true")
    else()
        set(EXEC_FLAG "false")
    endif()
    list(APPEND MANIFEST_ENTRIES "    {\"${REL_PATH}\", ${SYMBOL}, ${SYMBOL}_len, ${EXEC_FLAG}},")
endforeach()

set(MANIFEST_HEADER "${GENERATED_DIR}/embed_manifest.hpp")
file(WRITE "${MANIFEST_HEADER}" "#pragma once\n#include <cstddef>\n\n")
foreach(INCLUDE_LINE IN LISTS MANIFEST_INCLUDES)
    file(APPEND "${MANIFEST_HEADER}" "${INCLUDE_LINE}\n")
endforeach()
file(APPEND "${MANIFEST_HEADER}"
"\nstruct EmbeddedFile {\n"
"    const char* relative_path;\n"
"    const unsigned char* data;\n"
"    unsigned int size;\n"
"    bool executable;\n"
"};\n\n"
"inline const EmbeddedFile kEmbeddedFiles[] = {\n"
)
foreach(ENTRY IN LISTS MANIFEST_ENTRIES)
    file(APPEND "${MANIFEST_HEADER}" "${ENTRY}\n")
endforeach()
file(APPEND "${MANIFEST_HEADER}"
"};\n\n"
"inline constexpr std::size_t kEmbeddedFileCount = sizeof(kEmbeddedFiles) / sizeof(kEmbeddedFiles[0]);\n"
)

add_executable(cpp_igblast
    src/main.cpp
    src/tsv_mapper.cpp
    ${EMBED_HEADERS}
)

set_source_files_properties(${EMBED_HEADERS} PROPERTIES GENERATED TRUE)

target_include_directories(cpp_igblast PRIVATE "${GENERATED_DIR}" "${CMAKE_SOURCE_DIR}/src")
