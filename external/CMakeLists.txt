# set up the external libraries that aren't popular enough to be found on most
# package managers are included with the project (in the 'external' folder)

add_subdirectory("nanosvg")

find_package(pugixml QUIET)

if(NOT pugixml_FOUND)
  # Check if the pugixml submodule directory has content
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pugixml/CMakeLists.txt")
    add_subdirectory("pugixml")
  else()
    # Try to find pugixml in common locations (Homebrew, system)
    find_path(PUGIXML_INCLUDE_DIR
              NAMES pugixml.hpp
              PATHS
                /usr/local/include
                /opt/homebrew/include
                /usr/include)

    find_library(PUGIXML_LIBRARY
                 NAMES pugixml
                 PATHS
                   /usr/local/lib
                   /opt/homebrew/lib
                   /usr/lib)

    if(PUGIXML_INCLUDE_DIR AND PUGIXML_LIBRARY)
      message(STATUS "Found pugixml: ${PUGIXML_LIBRARY}")
      # Create an imported target for pugixml
      add_library(pugixml UNKNOWN IMPORTED)
      set_target_properties(pugixml PROPERTIES
        IMPORTED_LOCATION "${PUGIXML_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${PUGIXML_INCLUDE_DIR}")
    else()
      message(FATAL_ERROR "pugixml not found. Please either:\n"
                          "  1. Initialize git submodule: git submodule update --init\n"
                          "  2. Install via package manager:\n"
                          "     - macOS: brew install pugixml\n"
                          "     - Debian/Ubuntu: apt install libpugixml-dev\n"
                          "     - Fedora: dnf install pugixml-devel")
    endif()
  endif()
endif()
