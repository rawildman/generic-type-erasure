set(HEADERS generic-type-erasure.hpp)

add_library(GenericTypeErasure INTERFACE)
target_include_directories(GenericTypeErasure INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET GenericTypeErasure PROPERTY CXX_STANDARD 17)

