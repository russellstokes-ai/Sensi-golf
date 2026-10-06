add_library(sensigolf_core
    src/trace.cpp
)

target_include_directories(sensigolf_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(sensigolf_core PUBLIC cxx_std_17)

add_executable(sensigolf_core_contract_tests
    tests/core_contract_tests.cpp
)

target_link_libraries(sensigolf_core_contract_tests PRIVATE sensigolf_core)

add_test(
    NAME sensigolf_core_contract_tests
    COMMAND sensigolf_core_contract_tests
)
