# Creates the Testudo library and executable.

file(GLOB_RECURSE ENGINE_LIB_SRC "*.cpp")
list(REMOVE_ITEM ENGINE_LIB_SRC "testudo.cpp")

add_library(testudo_lib ${ENGINE_LIB_SRC})

add_executable(testudo "testudo.cpp")
target_link_libraries(testudo testudo_lib docopt)
