# DFM Pattern Library GUI

## Overview
The DFM Pattern Library GUI is a cross-platform application designed to provide a visual interface for browsing, searching, and visualizing patterns stored in a PostgreSQL/PostGIS database. The application enables users to explore pattern geometries and their associated metadata efficiently.

## Features
- **Pattern Browsing**: Navigate through stored patterns with pagination.
- **Pattern Search**: Search by name, metadata, and geometric properties.
- **Pattern Visualization**: Display pattern geometries with layer support.
- **Metadata Display**: View and edit pattern metadata.
- **Pattern Management**: Import, export, and delete patterns.
- **Similarity Search**: Find patterns similar to a selected pattern.

## Project Structure
```
pattern-library-gui
├── src
│   ├── controllers
│   ├── models
│   ├── ui
│   ├── utils
│   └── main.cpp
├── include
│   └── pattern-library
├── resources
├── tests
├── ui
├── CMakeLists.txt
├── pattern-library-gui.pro
└── README.md
```

## Build Instructions
1. Ensure you have CMake installed on your system.
2. Clone the repository:
   ```
   git clone <repository-url>
   cd pattern-library-gui
   ```
3. Create a build directory:
   ```
   mkdir build && cd build
   ```
4. Run CMake to configure the project:
   ```
   cmake ..
   ```
5. Build the project:
   ```
   make
   ```

## Running the Application
After building the project, you can run the application from the `bin` directory:
```
./pattern-library-gui
```

## Dependencies
- Qt framework for UI development.
- PostgreSQL for database management.
- OpenGL for rendering geometries.
- Boost libraries for additional functionality.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.