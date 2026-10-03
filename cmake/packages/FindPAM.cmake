
# - Try to find the PAM libraries
#
#  PAM_FOUND - pam was successfully found
#  PAM_INCLUDE_DIR - pam include directory
#  PAM_LIBRARIES - pam libraries

find_path(PAM_INCLUDE_DIR NAMES pam_appl.h PATH_SUFFIXES security pam)
find_library(PAM_LIBRARY pam)

find_package_handle_standard_args(PAM REQUIRED_VARS PAM_LIBRARY PAM_INCLUDE_DIR)
mark_as_advanced(PAM_INCLUDE_DIR PAM_LIBRARY)

if (PAM_FOUND)
    add_library(PAM::pam UNKNOWN IMPORTED GLOBAL)
    set_target_properties(PAM::pam PROPERTIES
            IMPORTED_LOCATION "${PAM_LIBRARY}")
    set_target_properties(PAM::pam PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${PAM_INCLUDE_DIR}")
endif ()
