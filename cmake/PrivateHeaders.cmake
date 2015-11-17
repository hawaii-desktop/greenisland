#.rst:
# PrivateHeaders
# ------------------
#
# Generate forwarding private headers.
#
# ::
#
#   private_headers(<forwarding_headers_var>
#       HEADERS <path/to/private/header> [<path/to/private/header> [...]]
#       [OUTPUT_DIR <output_dir>]
#
# For each private header path passed to HEADERS, a redirect will be created.
# The file locations of these generated headers will be stored in
# <forwarding_headers_var>.
#
# OUTPUT_DIR specifies where the files will be generated; this should be within
# the build directory. By default, ``${CMAKE_CURRENT_BINARY_DIR}`` will be used.
# This option can be used to avoid file conflicts.
#
# Example usage:
#
# .. code-block:: cmake
#
#   private_headers(
#       MyLib_PRIVATE_HEADERS
#       HEADERS
#           ${CMAKE_CURRENT_SOURCE_DIR}/header_p.h
#           ${CMAKE_CURRENT_BINARY_DIR}/generatedheader.h
#           # etc
#   )
#   install(FILES ${MyLib_PRIVATE_HEADERS}
#           DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MyLib/private
#           COMPONENT Devel)
#
# Since pre-1.0.0.

#=============================================================================
# Copyright 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file COPYING-CMAKE-SCRIPTS for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of greenisland, substitute the full
#  License text for the above reference.)

include(CMakeParseArguments)

function(PRIVATE_HEADERS private_headers_var)
    set(options)
    set(oneValueArgs OUTPUT_DIR)
    set(multiValueArgs HEADERS PRIVATE_HEADERS)
    cmake_parse_arguments(PH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(PH_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to GENERATE_HEADERS: ${PH_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT PH_HEADERS)
       message(FATAL_ERROR "Missing headers argument to GENERATE_HEADERS")
    endif()

    if(NOT PH_OUTPUT_DIR)
        set(PH_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    foreach(_filename ${PH_HEADERS})
        get_filename_component(_orig_filename "${_filename}" NAME)
        set(_private_header_file "${PH_OUTPUT_DIR}/private/${_orig_filename}")

        if(NOT EXISTS ${_private_header_file})
            file(WRITE ${_private_header_file} "#include \"${_filename}\"\n")
        endif()

        list(APPEND ${private_headers_var} "${_filename}")
    endforeach()

    set(${private_headers_var} ${${private_headers_var}} PARENT_SCOPE)
endfunction()
