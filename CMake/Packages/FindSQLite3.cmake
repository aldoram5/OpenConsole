# FindSQLite3.cmake
# Locate SQLite3 library
#
# This module defines:
# SQLite3_FOUND - System has SQLite3
# SQLite3_INCLUDE_DIRS - SQLite3 include directories
# SQLite3_LIBRARIES - Libraries needed to use SQLite3

find_path(SQLite3_INCLUDE_DIR
    NAMES sqlite3.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
)

find_library(SQLite3_LIBRARY
    NAMES sqlite3
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite3
    REQUIRED_VARS SQLite3_LIBRARY SQLite3_INCLUDE_DIR
)

if(SQLite3_FOUND)
    set(SQLite3_LIBRARIES ${SQLite3_LIBRARY})
    set(SQLite3_INCLUDE_DIRS ${SQLite3_INCLUDE_DIR})
    mark_as_advanced(SQLite3_INCLUDE_DIR SQLite3_LIBRARY)
endif()
