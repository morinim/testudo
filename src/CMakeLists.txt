cmake_minimum_required(VERSION 3.5)

project(TESTUDO)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

if (UNIX)
  add_definitions(-DUNIX)
endif (UNIX)
if (WIN32)
  add_definitions(-DWIN32)
endif (WIN32)

# The general idea is to use the default values and overwrite them only for
# specific, well experimented systems.
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "CLang")

set(NO_WARN_FLAGS "-w")

# -Wconversion -Wsign-conversion -Weffc++ are other interesting warning
# switches to try from time to time (they gives many false positives).
set(WARN_FLAGS "-Wpedantic" "-Wall" "-Wextra" "-Winvalid-pch" "-Wpedantic"
               "-Wformat=2" "-Wfloat-equal" "-Wshadow" "-Wdouble-promotion"
			   "-Wzero-as-null-pointer-constant")

set(OTHER_FLAGS "-pipe" "-march=native")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
set(CMAKE_CXX_FLAGS_DEBUG "-Og")
endif ()

endif ()

add_compile_options(${OTHER_FLAGS} ${WARN_FLAGS})

# Compiler must support the C++14 standard.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${TESTUDO_SOURCE_DIR})
include_directories(SYSTEM ${TESTUDO_SOURCE_DIR}/thirdparty/)

# Recurse into the following subdirectories (this does not actually cause
# another cmake executable to run. The same process will walk through the
# project's entire directory structure).
add_subdirectory(thirdparty/docopt)
add_subdirectory(engine)

enable_testing()
add_subdirectory(tests)
