cmake_minimum_required(VERSION 3.10)
project(pattern_library_gui VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic handling of Qt files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Qt libraries
find_package(Qt6 COMPONENTS Core Gui Widgets Sql REQUIRED)
find_package(OpenGL REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/controllers/DatabaseController.cpp
    src/controllers/PatternController.cpp
    src/models/PatternModel.cpp
    src/models/SearchModel.cpp
    src/ui/MainWindow.cpp
    src/ui/mainwindow.ui # Corrected UI file name
    src/ui/PatternBrowser.cpp
    src/ui/PatternViewer.cpp
    src/utils/GeometryUtils.cpp
    src/utils/ImportExport.cpp
    src/utils/FileHandler.cpp
)

# Header files
set(HEADERS
    src/controllers/DatabaseController.h
    src/controllers/PatternController.h
    src/models/PatternModel.h
    src/models/SearchModel.h
    src/ui/MainWindow.h
    src/ui/PatternBrowser.h
    src/ui/PatternViewer.h
    src/utils/GeometryUtils.h
    src/utils/ImportExport.h
    src/utils/FileHandler.h
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add include paths
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Try to build and link gdstk from source
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../gdstk")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gdstk ${CMAKE_BINARY_DIR}/gdstk)
    set(HAVE_GDSTK TRUE)
    set(GDSTK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../gdstk/include)
else()
    message(STATUS "GDSTK source not found - building without GDSTK support")
    set(HAVE_GDSTK FALSE)
endif()

# Configure header with GDSTK support flag
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE
        Qt6::Core 
        Qt6::Gui 
        Qt6::Widgets
        Qt6::Sql
        OpenGL::GL
)

# If GDSTK is available, link it
if(HAVE_GDSTK)
    target_link_libraries(${PROJECT_NAME} PRIVATE gdstk)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GDSTK_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_GDSTK)
endif()

# Option for building tests
option(BUILD_TESTS "Build tests" OFF)

# Only add test directories if BUILD_TESTS is ON
if(BUILD_TESTS)
    find_package(GTest)
    if(GTest_FOUND)
        enable_testing()
        add_subdirectory(tests/unit)
        add_subdirectory(tests/integration)
    else()
        message(STATUS "GTest not found - tests will not be built")
    endif()
endif()