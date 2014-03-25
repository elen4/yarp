# Copyright: (C) 2010 RobotCub Consortium
# Authors: Paul Fitzpatrick
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

include(GNUInstallDirs)

# Let's see what we built, and record it to facilitate in-tree
# ("uninstalled") use of YARP.
get_property(YARP_INCLUDE_DIRS GLOBAL PROPERTY YARP_TREE_INCLUDE_DIRS)
get_property(YARP_LINK_DIRS GLOBAL PROPERTY YARP_TREE_LINK_DIRS)
get_property(YARP_LIBS GLOBAL PROPERTY YARP_LIBS)
# Oops, cannot use YARP_DEFINES name, conflicts with an old variable
# that might be lurking in CMakeCache.txt as people upgrade.  Insert
# an "_ALL_" for now.
get_property(YARP_ALL_DEFINES GLOBAL PROPERTY YARP_DEFS)
get_property(YARP_HAS_MATH_LIB GLOBAL PROPERTY YARP_HAS_MATH_LIB)
message(STATUS "In-tree includes: ${YARP_INCLUDE_DIRS}")
message(STATUS "YARP libraries: ${YARP_LIBS}")

set(YARP_HAS_IDL FALSE)
if(ENABLE_yarpidl_thrift)
    set(YARP_HAS_IDL TRUE)
    set(YARP_IDL_BINARY_HINT ${CMAKE_BINARY_DIR}/bin)
    if (MSVC)
        set(YARP_IDL_BINARY_HINT "${YARP_IDL_BINARY_HINT};${YARP_IDL_BINARY_HINT}/Debug;${YARP_IDL_BINARY_HINT}/Release")
    endif(MSVC)
endif(ENABLE_yarpidl_thrift)

set(YARP_DEPENDENCY_FILE ${CMAKE_BINARY_DIR}/YARPDependencies.cmake)
set(YARP_DEPENDENCY_FILENAME YARPDependencies.cmake)
set(YARP_BINDINGS ${CMAKE_SOURCE_DIR}/bindings)

# Filter out from YARP_LIBRARIES all the plugins, yarpmod, yarpcar
# and all the other libraries that should not be linked by the user
set(YARP_LIBRARIES)
foreach(lib ${YARP_LIBS})
  if(NOT "${lib}" MATCHES "carrier$" AND
     NOT "${lib}" MATCHES "^yarp_" AND
     NOT "${lib}" STREQUAL "yarpcar" AND
     NOT "${lib}" STREQUAL "yarpmod" AND
     NOT "${lib}" STREQUAL "YARP_wire_rep_utils" AND
     NOT "${lib}" STREQUAL "YARP_manager")
    list(APPEND YARP_LIBRARIES ${lib})
  endif()
endforeach()

configure_file(${CMAKE_CURRENT_LIST_DIR}/template/YARPConfig.cmake.in
               ${CMAKE_BINARY_DIR}/YARPConfig.cmake @ONLY)
if (${CMAKE_VERSION} VERSION_LESS 2.8.8) # -> version is 2.8.7 (oldest supported)
  include(WriteBasicConfigVersionFile)
  write_basic_config_version_file(${CMAKE_BINARY_DIR}/YARPConfigVersion.cmake
                                     VERSION ${YARP_VERSION}
                                     COMPATIBILITY AnyNewerVersion )
else()
  include(CMakePackageConfigHelpers)
  WRITE_BASIC_PACKAGE_VERSION_FILE(${CMAKE_BINARY_DIR}/YARPConfigVersion.cmake VERSION ${YARP_VERSION} COMPATIBILITY AnyNewerVersion )
endif()
export(TARGETS ${YARP_LIBRARIES} FILE ${YARP_DEPENDENCY_FILE})

set(VERSIONED_LIB ${CMAKE_INSTALL_LIBDIR}/YARP-${YARP_VERSION})

# Set up a configuration file for installed use of YARP
set(YARP_DEPENDENCY_FILE ${CMAKE_INSTALL_PREFIX}/${VERSIONED_LIB}/YARP.cmake)
set(YARP_DEPENDENCY_FILENAME YARP.cmake)
set(YARP_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
set(YARP_MODULE_DIR ${CMAKE_INSTALL_PREFIX}/share/yarp/cmake)
set(YARP_IDL_BINARY_HINT ${CMAKE_INSTALL_PREFIX}/bin)
set(YARP_BINDINGS ${CMAKE_INSTALL_PREFIX}/share/yarp/bindings)

set(YARP_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

configure_file(${CMAKE_CURRENT_LIST_DIR}/template/YARPConfig.cmake.in
               ${CMAKE_BINARY_DIR}/YARPConfigForInstall.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/YARPConfigForInstall.cmake RENAME YARPConfig.cmake COMPONENT configuration DESTINATION ${VERSIONED_LIB})
install(FILES ${CMAKE_BINARY_DIR}/YARPConfigVersion.cmake COMPONENT configuration DESTINATION ${VERSIONED_LIB})
install(EXPORT YARP COMPONENT configuration DESTINATION ${VERSIONED_LIB})

foreach(lib ${YARP_LIBS})
    set_target_properties(${lib} PROPERTIES VERSION ${YARP_VERSION}
                                            SOVERSION ${YARP_GENERIC_SOVERSION})
endforeach(lib)
