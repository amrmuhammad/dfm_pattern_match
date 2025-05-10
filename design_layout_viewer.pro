QT += widgets
CONFIG += debug CXXFLAGS+="-O0 -g" QMAKE_CXXFLAGS+="-O0 -g" QMAKE_CFLAGS+="-O0 -g"

# For gdstk and its dependencies (Clipper, Qhull)
INCLUDEPATH += /home/amrmuhammad/dev/gdstk/include \
               /usr/include # For zlib, qhull if system-installed

# Adjust if your Clipper/Qhull headers are elsewhere, though gdstk usually includes them or they are standard.

LIBS += /home/amrmuhammad/dev/gdstk/build/src/libgdstk.a \
        /home/amrmuhammad/dev/gdstk/build/external/libclipper.a \
        -L/usr/local/lib -lqhull_r \
        -lz # For zlib

# Add Boost if needed for geometry, though gdstk might handle what you need for viewing.
# If Boost headers are not in default paths:
# INCLUDEPATH += /path/to/boost_1_XX_0
# If Boost libs need linking (unlikely for just Boost.Geometry header-only parts):
# LIBS += -L/path/to/boost_1_XX_0/stage/lib -lboost_system # (example)


SOURCES += design_layout_viewer.cpp
# HEADERS += design_layout_viewer.h # If you create a header file

TARGET = design_layout_viewer
