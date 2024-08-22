set(SOURCES tests.cpp)

add_executable(unit_tests ${SOURCES})
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain GenericTypeErasure)

include(CTest)
include(Catch)

catch_discover_tests(unit_tests)

