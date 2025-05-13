# FindGDSTK.cmake

# Find gdstk includes and library
find_path(GDSTK_INCLUDE_DIR
    NAMES gdstk.hpp
    PATH_SUFFIXES include gdstk
    PATHS
        /usr/local
        /usr
)

find_library(GDSTK_LIBRARY
    NAMES gdstk libgdstk
    PATH_SUFFIXES lib lib64
    PATHS
        /usr/local
        /usr
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GDSTK
    REQUIRED_VARS
        GDSTK_LIBRARY
        GDSTK_INCLUDE_DIR
)

if(GDSTK_FOUND)
    set(GDSTK_LIBRARIES ${GDSTK_LIBRARY})
    set(GDSTK_INCLUDE_DIRS ${GDSTK_INCLUDE_DIR})
endif()
