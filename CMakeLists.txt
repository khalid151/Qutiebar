cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(qutiebar)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

set(MAJOR 0)
set(MINOR 3)
set(PATCH 2)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/Version.h.in
            ${CMAKE_CURRENT_SOURCE_DIR}/include/Version.h)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

file(GLOB_RECURSE all_sources src/*.cpp)
file(GLOB_RECURSE all_headers include/*.h)
set(sources ${all_sources})
set(headers ${all_headers})

include_directories(include)
include_directories(${XCB_INCLUDE_DIR})
include_directories(${EWMH_INCLUDE_DIR})
include_directories(${ICCCM_INCLUDE_DIR})

find_package(XCB COMPONENTS XCB EWMH ICCCM REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5X11Extras REQUIRED)

find_package(Qt5DBus)
find_package(MPD)
find_package(Alsa)

set(res ${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc)

# Libraries list
list(APPEND Libs Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras ${XCB_LIBRARIES})

# Enable module if found
if(${MPD_FOUND})
    add_definitions(-DENABLE_MPD)
    list(APPEND Libs ${MPD_LIBRARIES})
else()
    list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils/MpdConnection.cpp)
    list(REMOVE_ITEM headers ${CMAKE_CURRENT_SOURCE_DIR}/include/Utils/MpdConnection.h)
endif(${MPD_FOUND})

if(${Alsa_FOUND})
    add_definitions(-DENABLE_ALSA)
    list(APPEND Libs ${Alsa_LIBRARIES})
else()
    list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/Data/Volume.cpp)
    list(REMOVE_ITEM headers ${CMAKE_CURRENT_SOURCE_DIR}/include/Data/Volume.h)
endif(${Alsa_FOUND})

if(${Qt5DBus_FOUND})
    add_definitions(-DENABLE_DBUS)
    list(APPEND Libs Qt5::DBus)
endif(${Qt5DBus_FOUND})

add_executable(${PROJECT_NAME} ${sources} ${headers} ${res})
target_link_libraries(${PROJECT_NAME} ${Libs})

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
