# Creates the tests.

file(GLOB TESTS_SRC *.cpp)
foreach (test_src ${TESTS_SRC})
  get_filename_component(test ${test_src} NAME_WE)
  add_executable(${test} ${test_src})
  target_link_libraries(${test} testudo_lib)
endforeach()

add_test(NAME UnitTests COMMAND unittest)
