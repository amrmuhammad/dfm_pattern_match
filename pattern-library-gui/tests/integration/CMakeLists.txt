# CMake configuration for integration tests

# Specify the minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(PatternLibraryGuiIntegrationTests)

# Find the required packages
find_package(Qt5 REQUIRED COMPONENTS Widgets Test)

# Include the main project directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the integration test executable
add_executable(integration_tests
    test_integration.cpp
)

# Link the main application and Qt libraries
target_link_libraries(integration_tests
    Qt5::Widgets
    Qt5::Test
    pattern_library_gui
)

# Enable testing
enable_testing()

# Add a test for the integration tests
add_test(NAME IntegrationTests COMMAND integration_tests)