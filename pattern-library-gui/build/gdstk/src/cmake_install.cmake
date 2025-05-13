# Install script for directory: /home/amrmuhammad/dev/dfm_pattern_match/gdstk/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/amrmuhammad/dev/dfm_pattern_match/pattern-library-gui/build/lib/libgdstk.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gdstk" TYPE FILE FILES
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/allocator.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/array.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/cell.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/clipper_tools.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/curve.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/flexpath.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/font.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/gdsii.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/gdstk.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/gdswriter.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/label.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/library.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/map.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/oasis.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/pathcommon.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/polygon.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/property.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/raithdata.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/rawcell.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/reference.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/repetition.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/robustpath.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/set.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/sort.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/style.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/tagmap.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/utils.hpp"
    "/home/amrmuhammad/dev/dfm_pattern_match/gdstk/include/gdstk/vec.hpp"
    )
endif()

