
cmake_minimum_required(VERSION 2.8.12)
# Every project needs a name.  We call this the "examples" project.
project(dface)

set(DLIB_NO_GUI_SUPPORT true)

if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-fPIC")
endif()

add_subdirectory(../dlib dlib_build)
add_library(dface SHARED dface.cpp)

if (WIN32)
  SET_TARGET_PROPERTIES(dface PROPERTIES LINK_FLAGS /DEF:${CMAKE_SOURCE_DIR}/dface.def)
endif()

target_link_libraries(dface dlib::dlib )

