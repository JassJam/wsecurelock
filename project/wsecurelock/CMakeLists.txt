include(${CMAKE_CURRENT_LIST_DIR}/Sources.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Dependencies.cmake)

register_executable(
        wsecurelock
        SOURCES ${SOURCES}
        HEADERS ${HEADERS}
        INCLUDE_DIRS ${INCLUDE_DIRS}
        NAMESPACE
        ${THIS_PROJECT_NAMESPACE}
        EXPORT_SET
        "${THIS_PROJECT_NAMESPACE}Targets"
        LINK_LIBS ${LINK_LIBS}
)
