#.rst:
# ECMWayland
# ----------
#
# This module provides the following functions to generate C protocol
# implementations:
#
#   - ``ecm_add_wayland_client_protocol``
#   - ``ecm_add_wayland_server_protocol``
#
# and their C++ counterparts:
#
#   - ``ecm_add_qtwayland_client_protocol``
#   - ``ecm_add_qtwayland_server_protocol``
#
# ::
#
#   ecm_add_wayland_client_protocol(<source_files_var>
#                                   PROTOCOL <xmlfile>
#                                   BASENAME <basename>)
#
# Generate Wayland client protocol files from ``<xmlfile>`` XML
# definition for the ``<basename>`` interface and append those files
# to ``<source_files_var>``.
#
# ::
#
#   ecm_add_wayland_server_protocol(<source_files_var>
#                                   PROTOCOL <xmlfile>
#                                   BASENAME <basename>)
#
# Generate Wayland server protocol files from ``<xmlfile>`` XML
# definition for the ``<basename>`` interface and append those files
# to ``<source_files_var>``.
#
# ::
#
#   ecm_add_qtwayland_client_protocol(<source_files_var>
#                                     PROTOCOL <xmlfile>
#                                     BASENAME <basename>
#                                     [PREFIX <prefix>])
#
# Generate C++ wrapper to Wayland client protocol files from ``<xmlfile>``
# XML definition for the ``<basename>`` interface and append those files
# to ``<source_files_var>``.  Pass the ``<prefix>`` argument if the interface
# names don't start with qt_ or wl_.
#
# ::
#
#   ecm_add_qtwayland_server_protocol(<source_files_var>
#                                     PROTOCOL <xmlfile>
#                                     BASENAME <basename>
#                                     [PREFIX <prefix>])
#
# Generate C++ wrapper to Wayland server protocol files from ``<xmlfile>``
# XML definition for the ``<basename>`` interface and append those files
# to ``<source_files_var>``.  Pass the ``<prefix>`` argument if the interface
# names don't start with qt_ or wl_.
#

#=============================================================================
# Copyright 2012-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

include(CMakeParseArguments)

function(ecm_add_wayland_client_protocol out_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown keywords given to ecm_add_wayland_client_protocol(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    # Find wayland-scanner
    find_program(wayland_scanner_executable NAMES wayland-scanner)
    if(NOT wayland_scanner_executable)
        message(FATAL_ERROR "The wayland-scanner executable has not been found on your system. You must install it.")
    endif()

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_client_header "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-client-protocol.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-protocol.c")

    add_custom_command(OUTPUT "${_client_header}"
        COMMAND ${wayland_scanner_executable} client-header < ${_infile} > ${_client_header}
        DEPENDS ${_infile} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${wayland_scanner_executable} code < ${_infile} > ${_code}
        DEPENDS ${_infile} ${_client_header} VERBATIM)

    list(APPEND ${out_var} "${_client_header}" "${_code}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()


function(ecm_add_wayland_server_protocol out_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown keywords given to ecm_add_wayland_server_protocol(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    # Find wayland-scanner
    find_program(wayland_scanner_executable NAMES wayland-scanner)
    if(NOT wayland_scanner_executable)
        message(FATAL_ERROR "The wayland-scanner executable has not been found on your system. You must install it.")
    endif()

    ecm_add_wayland_client_protocol(${out_var}
                                    PROTOCOL ${ARGS_PROTOCOL}
                                    BASENAME ${ARGS_BASENAME})

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_server_header "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-server-protocol.h")

    add_custom_command(OUTPUT "${_server_header}"
        COMMAND ${wayland_scanner_executable} server-header < ${_infile} > ${_server_header}
        DEPENDS ${_infile} VERBATIM)

    list(APPEND ${out_var} "${_server_header}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()


function(ecm_add_qtwayland_client_protocol out_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME PREFIX)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown keywords given to ecm_add_qtwayland_client_protocol(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    if(ARGS_PREFIX)
        set(_prefix ${ARGS_PREFIX})
    else()
        set(_prefix "")
    endif()

    # Find qtwaylandscanner
    find_program(qtwaylandscanner_executable NAMES qtwaylandscanner)
    if(NOT qtwaylandscanner_executable)
        message(FATAL_ERROR "The qtwaylandscanner executable has not been found on your system. Make sure QTDIR/bin is in your PATH.")
    endif()

    ecm_add_wayland_client_protocol(${out_var}
                                    PROTOCOL ${ARGS_PROTOCOL}
                                    BASENAME ${ARGS_BASENAME})

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_cheader "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-client-protocol.h")
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${ARGS_BASENAME}.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${ARGS_BASENAME}.cpp")

    add_custom_command(OUTPUT "${_header}"
        COMMAND ${qtwaylandscanner_executable} client-header ${_infile} "" ${_prefix} > ${_header}
        DEPENDS ${_infile} ${_cheader} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${qtwaylandscanner_executable} client-code ${_infile} "" ${_prefix} > ${_code}
        DEPENDS ${_infile} ${_header} VERBATIM)

    list(APPEND ${out_var} "${_code}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()


function(ecm_add_qtwayland_server_protocol out_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME PREFIX)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown keywords given to ecm_add_qtwayland_server_protocol(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    if(ARGS_PREFIX)
        set(_prefix ${ARGS_PREFIX})
    else()
        set(_prefix "")
    endif()

    # Find qtwaylandscanner
    find_program(qtwaylandscanner_executable NAMES qtwaylandscanner)
    if(NOT qtwaylandscanner_executable)
        message(FATAL_ERROR "The qtwaylandscanner executable has not been found on your system. Make sure QTDIR/bin is in your PATH.")
    endif()

    ecm_add_wayland_server_protocol(${out_var}
                                    PROTOCOL ${ARGS_PROTOCOL}
                                    BASENAME ${ARGS_BASENAME})

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-${ARGS_BASENAME}.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-${ARGS_BASENAME}.cpp")

    add_custom_command(OUTPUT "${_header}"
        COMMAND ${qtwaylandscanner_executable} server-header ${_infile} "" ${_prefix} > ${_header}
        DEPENDS ${_infile} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${qtwaylandscanner_executable} server-code ${_infile} "" ${_prefix} > ${_code}
        DEPENDS ${_infile} ${_header} VERBATIM)

    list(APPEND ${out_var} "${_code}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()
