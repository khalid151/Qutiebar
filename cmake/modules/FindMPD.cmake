find_package(PkgConfig)
pkg_check_modules(PC_MPD QUIET libmpdclient)
set(MPD_DEFINITIONS ${PC_MPD_CFLAGS_OTHER})

find_path(MPD_INCLUDE_DIR mpd/client.h
    HINTS ${PC_MPD_INCLUDEDIR} ${PC_MPD_INCLUDE_DIRS}
    PATH_SUFFIXES mpd)

find_library(MPD_LIBRARY NAMES mpdclient
    HINTS ${PC_MPD_LIBDIR} ${PC_MPD_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MPD DEFAULT_MSG MPD_LIBRARY MPD_INCLUDE_DIR)

mark_as_advanced(MPD_INCLUDE_DIR MPD_LIBRARY)
set(MPD_LIBRARIES ${MPD_LIBRARY})
set(MPD_INCLUDE_DIRS ${MPD_INCLUDE_DIR})
