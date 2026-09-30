include(${CMAKE_CURRENT_LIST_DIR}/Sources.cmake)

register_executable(
        wsecurelock
        SOURCES ${SOURCES}
        HEADERS ${HEADERS}
        INCLUDE_DIRS ${INCLUDE_DIRS}
        NAMESPACE
        ${THIS_PROJECT_NAMESPACE}
        EXPORT_SET
        "${THIS_PROJECT_NAMESPACE}Targets")
