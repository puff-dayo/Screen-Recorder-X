cmake_minimum_required(VERSION 3.10)
project(ScreenRecorder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLTK REQUIRED)
include_directories(${FLTK_INCLUDE_DIRS})

add_executable(screenrecorder 
    main.cpp 
    get_screen.cpp
)

target_link_libraries(screenrecorder PRIVATE ${FLTK_LIBRARIES})

find_package(X11 REQUIRED)
find_library(XFT_LIB Xft REQUIRED)
find_library(FONTCONFIG_LIB fontconfig REQUIRED)
find_library(XRENDER_LIB Xrender REQUIRED)
find_library(XCURSOR_LIB Xcursor REQUIRED)
find_library(XFIXES_LIB Xfixes REQUIRED)
find_library(XINERAMA_LIB Xinerama REQUIRED)

target_link_libraries(screenrecorder PRIVATE
    ${X11_LIBRARIES}
    ${XFT_LIB}
    ${FONTCONFIG_LIB}
    ${XRENDER_LIB}
    ${XCURSOR_LIB}
    ${XFIXES_LIB}
    ${XINERAMA_LIB}
)

execute_process(
    COMMAND fltk-config --use-images --cxxflags
    OUTPUT_VARIABLE FLTK_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_options(screenrecorder PRIVATE ${FLTK_CXXFLAGS})

execute_process(
    COMMAND fltk-config --use-images --ldflags
    OUTPUT_VARIABLE FLTK_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_link_options(screenrecorder PRIVATE ${FLTK_LDFLAGS})
