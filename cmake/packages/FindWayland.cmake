# FindWayland.cmake - component based Wayland finder
#
# Usage:
#   find_package(Wayland REQUIRED COMPONENTS Client SessionLock)
#   target_link_libraries(app PRIVATE Wayland::Client Wayland::SessionLock)
#
# Library components  -> imported targets
#   Client      Wayland::Client       wayland-client
#   Server      Wayland::Server       wayland-server
#   SessionLock Wayland::SessionLock  wayland-protocols
#   EGL         Wayland::EGL          wayland-egl
#   EGLBackend  Wayland::EGLBackend   wayland-egl-backend
#   Cursor      Wayland::Cursor       wayland-cursor
#
# Tool components
#   Scanner     Wayland::Scanner      wayland-scanner executable
#   Protocols   (no target)           WAYLAND_PROTOCOLS_DIR (wayland-protocols)
#
if (NOT WIN32)
    include(FindPackageHandleStandardArgs)

    find_package(PkgConfig QUIET)
    if (NOT PKG_CONFIG_EXECUTABLE)
        find_program(PKG_CONFIG_EXECUTABLE NAMES pkg-config pkgconf)
    endif ()

    function(_wayland_pc_var out pkg var)
        set(${out} "" PARENT_SCOPE)
        if (PKG_CONFIG_EXECUTABLE)
            execute_process(
                    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${var} ${pkg}
                    OUTPUT_VARIABLE _v
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
                    RESULT_VARIABLE _r)
            if (_r EQUAL 0)
                set(${out} "${_v}" PARENT_SCOPE)
            endif ()
        endif ()
    endfunction()

    # component tables
    set(_WAYLAND_LIB_COMPONENTS   Client Server EGL EGLBackend Cursor)
    set(_WAYLAND_PROTO_COMPONENTS SessionLock XdgShell)

    set(_WAYLAND_Client_PC     wayland-client)
    set(_WAYLAND_Client_HEADER wayland-client.h)
    set(_WAYLAND_Client_LIB    wayland-client)
    set(_WAYLAND_Server_PC     wayland-server)
    set(_WAYLAND_Server_HEADER wayland-server.h)
    set(_WAYLAND_Server_LIB    wayland-server)
    set(_WAYLAND_EGL_PC        wayland-egl)
    set(_WAYLAND_EGL_HEADER    wayland-egl.h)
    set(_WAYLAND_EGL_LIB       wayland-egl)
    set(_WAYLAND_EGLBackend_PC     wayland-egl-backend)
    set(_WAYLAND_EGLBackend_HEADER wayland-egl-backend.h)
    set(_WAYLAND_EGLBackend_LIB    "")           # header-only
    set(_WAYLAND_Cursor_PC     wayland-cursor)
    set(_WAYLAND_Cursor_HEADER wayland-cursor.h)
    set(_WAYLAND_Cursor_LIB    wayland-cursor)

    set(_WAYLAND_SessionLock_XML_REL staging/ext-session-lock/ext-session-lock-v1.xml)
    set(_WAYLAND_SessionLock_XMLVAR  WAYLAND_EXT_SESSION_LOCK_XML)
    set(_WAYLAND_XdgShell_XML_REL    stable/xdg-shell/xdg-shell.xml)
    set(_WAYLAND_XdgShell_XMLVAR     WAYLAND_XDG_SHELL_XML)

    if (NOT Wayland_FIND_COMPONENTS)
        set(Wayland_FIND_COMPONENTS Client)
        set(Wayland_FIND_REQUIRED_Client ${Wayland_FIND_REQUIRED})
    endif ()

    set(WAYLAND_INCLUDE_DIR "")
    set(WAYLAND_LIBRARIES   "")

    # library components
    foreach (_comp IN LISTS _WAYLAND_LIB_COMPONENTS)
        list(FIND Wayland_FIND_COMPONENTS ${_comp} _idx)
        if (_idx EQUAL -1 AND NOT _comp STREQUAL "Client")
            continue()
        endif ()

        string(TOUPPER ${_comp} _U)
        set(_pc  ${_WAYLAND_${_comp}_PC})
        set(_hdr ${_WAYLAND_${_comp}_HEADER})
        set(_lib ${_WAYLAND_${_comp}_LIB})

        if (PKG_CONFIG_EXECUTABLE AND COMMAND pkg_check_modules)
            pkg_check_modules(PKG_WAYLAND_${_U} QUIET ${_pc})
        endif ()

        find_path(WAYLAND_${_U}_INCLUDE_DIR NAMES ${_hdr}
                HINTS ${PKG_WAYLAND_${_U}_INCLUDE_DIRS})
        if (_lib)
            find_library(WAYLAND_${_U}_LIBRARIES NAMES ${_lib}
                    HINTS ${PKG_WAYLAND_${_U}_LIBRARY_DIRS})
        endif ()
        mark_as_advanced(WAYLAND_${_U}_INCLUDE_DIR WAYLAND_${_U}_LIBRARIES)

        set(_ok FALSE)
        if (WAYLAND_${_U}_INCLUDE_DIR)
            if (NOT _lib OR WAYLAND_${_U}_LIBRARIES)
                set(_ok TRUE)
            endif ()
        endif ()

        if (_ok)
            set(Wayland_${_comp}_FOUND TRUE)
            set(WAYLAND_${_U}_FOUND TRUE)
            list(APPEND WAYLAND_INCLUDE_DIR ${WAYLAND_${_U}_INCLUDE_DIR})
            if (_lib)
                list(APPEND WAYLAND_LIBRARIES ${WAYLAND_${_U}_LIBRARIES})
            endif ()

            if (NOT TARGET Wayland::${_comp})
                if (_lib)
                    add_library(Wayland::${_comp} UNKNOWN IMPORTED GLOBAL)
                    set_target_properties(Wayland::${_comp} PROPERTIES
                            IMPORTED_LOCATION "${WAYLAND_${_U}_LIBRARIES}")
                else ()
                    add_library(Wayland::${_comp} INTERFACE IMPORTED GLOBAL)
                endif ()
                set_target_properties(Wayland::${_comp} PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_${_U}_INCLUDE_DIR}")
                if (_comp STREQUAL "Cursor" AND TARGET Wayland::Client)
                    set_property(TARGET Wayland::Cursor APPEND PROPERTY
                            INTERFACE_LINK_LIBRARIES Wayland::Client)
                endif ()
            endif ()
        else ()
            set(Wayland_${_comp}_FOUND FALSE)
            set(WAYLAND_${_U}_FOUND FALSE)
        endif ()
    endforeach ()
    if (WAYLAND_INCLUDE_DIR)
        list(REMOVE_DUPLICATES WAYLAND_INCLUDE_DIR)
    endif ()
    if (PKG_WAYLAND_CLIENT_CFLAGS)
        set(WAYLAND_DEFINITIONS ${PKG_WAYLAND_CLIENT_CFLAGS})
    endif ()

    # wayland-scanner dir, can be overwritten with -DWAYLAND_SCANNER=/path-to/wayland-scanner
    if (WAYLAND_SCANNER AND NOT EXISTS "${WAYLAND_SCANNER}")
        unset(WAYLAND_SCANNER CACHE)
    endif ()
    _wayland_pc_var(_scanner_pc wayland-scanner wayland_scanner)
    _wayland_pc_var(_wl_prefix  wayland-client  prefix)
    set(_scanner_hints "")
    if (_scanner_pc)
        get_filename_component(_scanner_dir "${_scanner_pc}" DIRECTORY)
        list(APPEND _scanner_hints "${_scanner_dir}")
    endif ()
    if (_wl_prefix)
        list(APPEND _scanner_hints "${_wl_prefix}/bin")
    endif ()
    find_program(WAYLAND_SCANNER NAMES wayland-scanner
            HINTS ${_scanner_hints}
            PATHS /usr/bin /usr/local/bin)
    if (NOT WAYLAND_SCANNER AND _scanner_pc AND EXISTS "${_scanner_pc}")
        set(WAYLAND_SCANNER "${_scanner_pc}")
    endif ()
    mark_as_advanced(WAYLAND_SCANNER)

    if (WAYLAND_SCANNER)
        set(Wayland_Scanner_FOUND TRUE)
        if (NOT TARGET Wayland::Scanner)
            add_executable(Wayland::Scanner IMPORTED GLOBAL)
            set_target_properties(Wayland::Scanner PROPERTIES
                    IMPORTED_LOCATION "${WAYLAND_SCANNER}")
        endif ()
    else ()
        set(Wayland_Scanner_FOUND FALSE)
    endif ()

    # wayland-protocols dir, can be overwritten with -DWAYLAND_PROTOCOLS_DIR=/usr/share/wayland-protocols
    if (NOT WAYLAND_PROTOCOLS_DIR)
        _wayland_pc_var(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
    endif ()
    if (NOT WAYLAND_PROTOCOLS_DIR)
        _wayland_pc_var(_wp_prefix wayland-protocols prefix)
        if (_wp_prefix)
            set(WAYLAND_PROTOCOLS_DIR "${_wp_prefix}/share/wayland-protocols")
        endif ()
    endif ()
    if (NOT WAYLAND_PROTOCOLS_DIR)
        foreach (_p ${CMAKE_PREFIX_PATH} /usr /usr/local)
            if (IS_DIRECTORY "${_p}/share/wayland-protocols")
                set(WAYLAND_PROTOCOLS_DIR "${_p}/share/wayland-protocols")
                break()
            endif ()
        endforeach ()
    endif ()
    if (WAYLAND_PROTOCOLS_DIR AND IS_DIRECTORY "${WAYLAND_PROTOCOLS_DIR}")
        get_filename_component(WAYLAND_PROTOCOLS_DIR "${WAYLAND_PROTOCOLS_DIR}" ABSOLUTE)
        set(Wayland_Protocols_FOUND TRUE)
    else ()
        set(Wayland_Protocols_FOUND FALSE)
    endif ()

    # generated protocol components
    foreach (_comp IN LISTS _WAYLAND_PROTO_COMPONENTS)
        list(FIND Wayland_FIND_COMPONENTS ${_comp} _idx)
        if (_idx EQUAL -1)
            continue()
        endif ()

        set(_rel    "${_WAYLAND_${_comp}_XML_REL}")
        set(_xmlvar "${_WAYLAND_${_comp}_XMLVAR}")
        get_filename_component(_reldir  "${_rel}" DIRECTORY)
        get_filename_component(_xmlname "${_rel}" NAME)

        if (${_xmlvar} AND NOT EXISTS "${${_xmlvar}}")
            unset(${_xmlvar} CACHE)      # stale cache entry (file moved/removed)
        endif ()
        find_file(${_xmlvar} NAMES ${_xmlname}
                PATHS "${WAYLAND_PROTOCOLS_DIR}/${_reldir}"
                "/usr/share/wayland-protocols/${_reldir}"
                "/usr/local/share/wayland-protocols/${_reldir}"
                "${CMAKE_CURRENT_LIST_DIR}"             # bundled: next to this file
                NO_DEFAULT_PATH)
        mark_as_advanced(${_xmlvar})
        set(_xml "${${_xmlvar}}")

        if (WAYLAND_SCANNER AND _xml)
            set(Wayland_${_comp}_FOUND TRUE)

            # The generated protocol glue is C. Make sure C is enabled even
            # in a C++-only project, otherwise the .c file is never compiled.
            get_property(_wl_langs GLOBAL PROPERTY ENABLED_LANGUAGES)
            list(FIND _wl_langs C _wl_c)
            if (_wl_c EQUAL -1)
                enable_language(C)
            endif ()

            if (NOT TARGET Wayland::${_comp})
                string(TOLOWER ${_comp} _lc)
                get_filename_component(_base "${_xml}" NAME_WE)
                set(_gen "${CMAKE_CURRENT_BINARY_DIR}/wayland_generated")
                set(_h   "${_gen}/${_base}-client-protocol.h")
                set(_c   "${_gen}/${_base}-protocol.c")
                set(_tgt wayland_protocol_${_lc})
                file(MAKE_DIRECTORY "${_gen}")

                add_custom_command(
                        OUTPUT  "${_h}"
                        COMMAND "${WAYLAND_SCANNER}" client-header "${_xml}" "${_h}"
                        DEPENDS "${_xml}"
                        VERBATIM)
                add_custom_command(
                        OUTPUT  "${_c}"
                        COMMAND "${WAYLAND_SCANNER}" private-code "${_xml}" "${_c}"
                        DEPENDS "${_xml}"
                        VERBATIM)
                add_custom_target(${_tgt}_gen DEPENDS "${_h}" "${_c}")

                add_library(${_tgt} INTERFACE)
                add_dependencies(${_tgt} ${_tgt}_gen)
                target_sources(${_tgt} INTERFACE "${_h}" "${_c}")
                target_include_directories(${_tgt} INTERFACE "${_gen}")
                if (TARGET Wayland::Client)
                    target_link_libraries(${_tgt} INTERFACE Wayland::Client)
                endif ()

                add_library(Wayland::${_comp} ALIAS ${_tgt})
                if (_comp STREQUAL "SessionLock")
                    add_library(wayland_protocol ALIAS ${_tgt})
                endif ()
            endif ()
        else ()
            set(Wayland_${_comp}_FOUND FALSE)
            if (NOT Wayland_FIND_QUIETLY)
                set(_bundled "${CMAKE_CURRENT_LIST_DIR}/${_xmlname}")
                message(STATUS "Wayland ${_comp} NOT found. Details:")
                message(STATUS "  module file    : ${CMAKE_CURRENT_LIST_FILE}")
                message(STATUS "  wayland-scanner: '${WAYLAND_SCANNER}'")
                message(STATUS "  protocols dir  : '${WAYLAND_PROTOCOLS_DIR}'")
                message(STATUS "  ${_xmlvar}: '${_xml}'")
                if (EXISTS "${_bundled}")
                    message(STATUS "  bundled xml    : found at ${_bundled}")
                else ()
                    message(STATUS "  bundled xml    : MISSING, expected exactly at ${_bundled}")
                endif ()
            endif ()
        endif ()
    endforeach ()

    if (NOT Wayland_FIND_QUIETLY)
        message(STATUS "Wayland lookup: scanner='${WAYLAND_SCANNER}' "
                "protocols_dir='${WAYLAND_PROTOCOLS_DIR}'")
    endif ()

    set(_WAYLAND_COMPONENTS_RESOLVED TRUE)
    find_package_handle_standard_args(Wayland
            REQUIRED_VARS _WAYLAND_COMPONENTS_RESOLVED
            HANDLE_COMPONENTS)

    mark_as_advanced(WAYLAND_PROTOCOLS_DIR)
endif ()
