#=============================================================================
# Copyright 2012-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#
# greenisland_install_shell(shell srcpath)
#
# Installs a Green Island shell
#
# \param shell the shell name (something like org.vendor.shell)
# \param srcpath the source path to install from
#
macro(greenisland_install_shell shell srcpath)
    install(DIRECTORY ${srcpath}/
            DESTINATION ${DATA_INSTALL_DIR}/greenisland/shells/${shell}
            PATTERN .svn EXCLUDE
            PATTERN CMakeLists.txt EXCLUDE
            PATTERN Messages.sh EXCLUDE
            PATTERN dummydata EXCLUDE)
endmacro()


include(CMakeParseArguments)

#
#   greenisland_add_client_protocol(<source_files_var>
#                                   PROTOCOL <xmlfile>
#                                   BASENAME <basename>
#                                   [PREFIX <prefix>])
#
# Generate C++ wrapper to Wayland client protocol files from ``<xmlfile>``
# XML definition for the ``<basename>`` interface and append those files
# to ``<source_files_var>``.  Pass the ``<prefix>`` argument if the interface
# names don't start with ``qt_`` or ``wl_``.
#
# WaylandScanner is required and will be searched for.
#
function(greenisland_add_client_protocol out_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME PREFIX)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown keywords given to greenisland_add_client_protocol(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    set(_prefix "${ARGS_PREFIX}")

    find_package(WaylandScanner REQUIRED QUIET)
    ecm_add_wayland_client_protocol(${out_var}
                                    PROTOCOL ${ARGS_PROTOCOL}
                                    BASENAME ${ARGS_BASENAME})

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_cheader "${CMAKE_CURRENT_BINARY_DIR}/wayland-${ARGS_BASENAME}-client-protocol.h")
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${ARGS_BASENAME}.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/qwayland-${ARGS_BASENAME}.cpp")

    set_source_files_properties(${_header} ${_code} GENERATED)

    add_custom_command(OUTPUT "${_header}"
        COMMAND ${GreenIsland_WAYLAND_SCANNER_EXECUTABLE} client-header ${_infile} "" ${_prefix} > ${_header}
        DEPENDS ${_infile} ${_cheader} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${GreenIsland_WAYLAND_SCANNER_EXECUTABLE} client-code ${_infile} "" ${_prefix} > ${_code}
        DEPENDS ${_infile} ${_header} VERBATIM)

    list(APPEND ${out_var} "${_code}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()

#
#   greenisland_add_server_protocol(<source_files_var>
#                                   PROTOCOL <xmlfile>
#                                   BASENAME <basename>
#                                   [PREFIX <prefix>])
#
# Generate C++ wrapper to Wayland server protocol files from ``<xmlfile>``
# XML definition for the ``<basename>`` interface and append those files
# to ``<source_files_var>``.  Pass the ``<prefix>`` argument if the interface
# names don't start with ``qt_`` or ``wl_``.
#
# WaylandScanner is required and will be searched for.
#
function(greenisland_add_server_protocol out_var)
    # Parse arguments
    set(oneValueArgs PROTOCOL BASENAME PREFIX)
    cmake_parse_arguments(ARGS "" "${oneValueArgs}" "" ${ARGN})

    if(ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown keywords given to greenisland_add_server_protocol(): \"${ARGS_UNPARSED_ARGUMENTS}\"")
    endif()

    set(_prefix "${ARGS_PREFIX}")

    find_package(WaylandScanner REQUIRED QUIET)
    ecm_add_wayland_server_protocol(${out_var}
                                    PROTOCOL ${ARGS_PROTOCOL}
                                    BASENAME ${ARGS_BASENAME})

    get_filename_component(_infile ${ARGS_PROTOCOL} ABSOLUTE)
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-${ARGS_BASENAME}.h")
    set(_code "${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-${ARGS_BASENAME}.cpp")

    set_source_files_properties(${_header} ${_code} GENERATED)

    add_custom_command(OUTPUT "${_header}"
        COMMAND ${GreenIsland_WAYLAND_SCANNER_EXECUTABLE} server-header ${_infile} "" ${_prefix} > ${_header}
        DEPENDS ${_infile} VERBATIM)

    add_custom_command(OUTPUT "${_code}"
        COMMAND ${GreenIsland_WAYLAND_SCANNER_EXECUTABLE} server-code ${_infile} "" ${_prefix} > ${_code}
        DEPENDS ${_infile} ${_header} VERBATIM)

    list(APPEND ${out_var} "${_code}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()
