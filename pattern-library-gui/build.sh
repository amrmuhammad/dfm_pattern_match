#!/bin/bash

# Function to check if a package is installed
check_package() {
    if ! dpkg -l | grep -q "^ii  $1"; then
        echo "Installing $1..."
        sudo apt-get install -y "$1"
    fi
}

# Install dependencies
echo "Checking and installing dependencies..."
check_package "qt5-default"
check_package "libqt5sql5-psql"
check_package "postgresql"
check_package "postgresql-contrib"
check_package "libgdstk-dev"

# Initialize database if not already done
if ! psql -lqt | cut -d \| -f 1 | grep -qw "pattern_library"; then
    echo "Initializing database..."
    ./scripts/init_db.sh
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure CMake project
cmake .. -DBUILD_TESTS=OFF

# Build the project
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Running the application..."
    ./pattern_library_gui
else
    echo "Build failed!"
    exit 1
fi
