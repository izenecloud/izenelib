#
# User need append izenelib install prefix to CMAKE_PREFIX_PATH
# 
# This is the topmost install prefix, where izenelib-version, shared,
# cmake can be found as subdirectories in it.
#
# CMAKE_PREFIX_PATH can be set through cmake variable
# (-DCMAKE_PREFIX_PATH="/path")
#
# or set as environment variable:
#
#  export CMAKE_PREFIX_PATH="/path"
#
# You can add the export in ~/.bashrc (user level) or /etc/profile
# (system level)
#
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (hello)

FIND_PACKAGE(izenelib 1.0.0 REQUIRED COMPONENTS febird)
INCLUDE_DIRECTORIES(${izenelib_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(hello febird_shared)

ADD_EXECUTABLE(hello hello.cpp)
