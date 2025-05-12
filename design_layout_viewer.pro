QT += widgets
CONFIG += debug CXXFLAGS+="-O0 -g" QMAKE_CXXFLAGS+="-O0 -g" QMAKE_CFLAGS+="-O0 -g"
CONFIG += c++14 # Use C++14 standard

# For gdstk and its dependencies (Clipper, Qhull)
INCLUDEPATH += /home/amrmuhammad/dev/gdstk/include \
               /usr/include # For zlib, qhull if system-installed

# gBolt related paths temporarily removed for testing compilation
# INCLUDEPATH += /home/amrmuhammad/dev/dfm_pattern_match/gBolt/include \
#                /home/amrmuhammad/dev/dfm_pattern_match/gBolt/extern/jemalloc \
#                /home/amrmuhammad/dev/dfm_pattern_match/gBolt/extern/jemalloc/include

LIBS += /home/amrmuhammad/dev/gdstk/build/src/libgdstk.a \
        /home/amrmuhammad/dev/gdstk/build/external/libclipper.a \
        -L/usr/local/lib -lqhull_r \~/…/dfm_pattern_match $ make
￼
Makefile:3740: warning: overriding recipe for target 'extent.o'
Makefile:1762: warning: ignoring old recipe for target 'extent.o'
Makefile:3858: warning: overriding recipe for target 'background_thread.o'
Makefile:1215: warning: ignoring old recipe for target 'background_thread.o'
Makefile:3864: warning: overriding recipe for target 'base.o'
Makefile:1304: warning: ignoring old recipe for target 'base.o'
Makefile:3876: warning: overriding recipe for target 'bitmap.o'
Makefile:1480: warning: ignoring old recipe for target 'bitmap.o'
Makefile:3879: warning: overriding recipe for target 'ckh.o'
Makefile:1568: warning: ignoring old recipe for target 'ckh.o'
Makefile:3889: warning: overriding recipe for target 'div.o'
Makefile:1673: warning: ignoring old recipe for target 'div.o'
Makefile:3906: warning: overriding recipe for target 'hash.o'
Makefile:2028: warning: ignoring old recipe for target 'hash.o'
Makefile:3938: warning: overriding recipe for target 'hook.o'
Makefile:2073: warning: ignoring old recipe for target 'hook.o'
Makefile:3984: warning: overriding recipe for target 'log.o'
Makefile:2431: warning: ignoring old recipe for target 'log.o'
Makefile:4019: warning: overriding recipe for target 'malloc_io.o'
Makefile:2519: warning: ignoring old recipe for target 'malloc_io.o'
Makefile:4022: warning: overriding recipe for target 'math.o'
Makefile:3788: warning: ignoring old recipe for target 'math.o'
Makefile:4025: warning: overriding recipe for target 'mq.o'
Makefile:3791: warning: ignoring old recipe for target 'mq.o'
Makefile:4028: warning: overriding recipe for target 'mtx.o'
Makefile:3794: warning: ignoring old recipe for target 'mtx.o'
Makefile:4031: warning: overriding recipe for target 'nstime.o'
Makefile:2784: warning: ignoring old recipe for target 'nstime.o'
Makefile:4037: warning: overriding recipe for target 'pages.o'
Makefile:2872: warning: ignoring old recipe for target 'pages.o'
Makefile:4043: warning: overriding recipe for target 'prng.o'
Makefile:2960: warning: ignoring old recipe for target 'prng.o'
Makefile:4115: warning: overriding recipe for target 'rtree.o'
Makefile:3137: warning: ignoring old recipe for target 'rtree.o'
Makefile:4118: warning: overriding recipe for target 'sc.o'
Makefile:3158: warning: ignoring old recipe for target 'sc.o'
Makefile:4132: warning: overriding recipe for target 'SFMT.o'
Makefile:3797: warning: ignoring old recipe for target 'SFMT.o'
Makefile:4147: warning: overriding recipe for target 'stats.o'
Makefile:3248: warning: ignoring old recipe for target 'stats.o'
Makefile:4153: warning: overriding recipe for target 'test_hooks.o'
Makefile:3367: warning: ignoring old recipe for target 'test_hooks.o'
Makefile:4157: warning: overriding recipe for target 'ticker.o'
Makefile:3455: warning: ignoring old recipe for target 'ticker.o'
Makefile:4160: warning: overriding recipe for target 'tsd.o'
Makefile:3543: warning: ignoring old recipe for target 'tsd.o'
Makefile:4163: warning: overriding recipe for target 'witness.o'
Makefile:3631: warning: ignoring old recipe for target 'witness.o'
g++ -c -pipe -O2 -std=gnu++11 -Wall -Wextra -D_REENTRANT -fPIC -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I. -I. -IgBolt/include -IgBolt/extern/jemalloc -IgBolt/extern/jemalloc/include -I../gdstk/include -I/usr/include -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtGui -I/usr/include/x86_64-linux-gnu/qt5/QtCore -I. -I/usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++ -o dfm_pattern_capture.o dfm_pattern_capture.cpp
In file included from /usr/include/boost/geometry.hpp:17,
                 from dfm_pattern_capture.cpp:7:
/usr/include/boost/geometry/geometry.hpp:26:2: error: #error "Use C++14 or higher to compile Boost.Geometry, or use Boost 1.72 or lower."
   26 | #error "Use C++14 or higher to compile Boost.Geometry, or use Boost 1.72 or lower."
      |  ^~~~~
In file included from /usr/include/boost/multiprecision/cpp_int.hpp:17,
                 from /usr/include/boost/multiprecision/cpp_bin_float.hpp:15,
                 from /usr/include/boost/geometry/core/coordinate_promotion.hpp:16,
                 from /usr/include/boost/geometry/core/radian_access.hpp:30,
                 from /usr/include/boost/geometry/geometry.hpp:50:
/usr/include/boost/multiprecision/detail/standalone_config.hpp:36:7: warning: #warning "The minimum language standard to use Boost.Math will be C++14 starting in July 2023 (Boost 1.82 release)" [-Wcpp]
   36 |  #    warning "The minimum language standard to use Boost.Math will be C++14 starting in July 2023 (Boost 1.82 release)"
      |       ^~~~~~~
In file included from /usr/include/boost/math/cstdfloat/cstdfloat_types.hpp:17,
                 from /usr/include/boost/cstdfloat.hpp:18,
                 from /usr/include/boost/multiprecision/detail/float128_functions.hpp:15,
                 from /usr/include/boost/multiprecision/detail/fpclassify.hpp:13,
                 from /usr/include/boost/multiprecision/detail/default_ops.hpp:15,
                 from /usr/include/boost/multiprecision/detail/generic_interconvert.hpp:12,
                 from /usr/include/boost/multiprecision/number.hpp:12,
                 from /usr/include/boost/multiprecision/cpp_int.hpp:19:
/usr/include/boost/math/tools/config.hpp:23:6: warning: #w
        -lz # For zlib

# Add Boost if needed for geometry, though gdstk might handle what you need for viewing.
# If Boost headers are not in default paths:
# INCLUDEPATH += /path/to/boost_1_XX_0
# If Boost libs need linking (unlikely for just Boost.Geometry header-only parts):
# LIBS += -L/path/to/boost_1_XX_0/stage/lib -lboost_system # (example)


HEADERS += layoutwidget.h

# Conditional build configuration
CONFIG(testcase) {
    QT += testlib
    TARGET = testlayoutwidget
    # Sources for the test build:
    # layoutwidget.cpp (the class being tested)
    # testlayoutwidget.cpp (the test code itself)
    # design_layout_viewer.cpp (contains main() for QApplication, often needed by QtTest)
    SOURCES = layoutwidget.cpp testlayoutwidget.cpp design_layout_viewer.cpp
    SOURCES -= dfm_pattern_capture.cpp # Explicitly remove from test build sources
    # If your testlayoutwidget.cpp has its own main(), you might remove design_layout_viewer.cpp from here.
    # HEADERS += testlayoutwidget.h # Uncomment if you have a testlayoutwidget.h
} else {
    # Application build
    TARGET = design_layout_viewer
    # Sources for the application build:
    SOURCES = design_layout_viewer.cpp layoutwidget.cpp dfm_pattern_capture.cpp
    # Add other application-specific sources and headers here if any
}

