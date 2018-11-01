find_package(PkgConfig)
pkg_check_modules(PC_Alsa QUIET alsa)
set(Alsa_DEFINITIONS ${PC_Alsa_CFLAGS_OTHER})

find_path(Alsa_INCLUDE_DIR alsa/mixer.h
    HINTS ${PC_Alsa_INCLUDEDIR} ${PC_Alsa_INCLUDE_DIRS}
    PATH_SUFFIXES alsa)

find_library(Alsa_LIBRARY NAMES asound
    HINTS ${PC_Alsa_LIBDIR} ${PC_Alsa_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Alsa DEFAULT_MSG Alsa_LIBRARY Alsa_INCLUDE_DIR)

mark_as_advanced(Alsa_INCLUDE_DIR Alsa_LIBRARY)
set(Alsa_LIBRARIES ${Alsa_LIBRARY})
set(Alsa_INCLUDE_DIRS ${Alsa_INCLUDE_DIR})
