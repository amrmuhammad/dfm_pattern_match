# CMake configuration for unit tests

# Include the Google Test framework
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Create a test executable
add_executable(unit_tests
    test_main.cpp
    DatabaseController_test.cpp
    PatternController_test.cpp
    PatternModel_test.cpp
    SearchModel_test.cpp
)

# Link the test executable with Google Test and the main application
target_link_libraries(unit_tests ${GTEST_LIBRARIES} pthread pattern_library_gui)

# Enable testing
enable_testing()

# Add tests
add_test(NAME UnitTests COMMAND unit_tests)