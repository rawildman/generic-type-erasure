set(SOURCES test-generic-type-erasure.cpp 
            test-type-helpers.cpp
            test-type-map.cpp
            test-examples.cpp)

add_executable(unit_tests ${SOURCES})
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain GenericTypeErasure)
target_compile_options(unit_tests PRIVATE ${WARNINGS})

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
