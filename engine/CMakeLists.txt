add_library(sensigolf_core
    src/trace.cpp
    src/classic_shot_model.cpp
    src/classic_course_resources.cpp
    src/classic_terrain.cpp
    src/classic_hole_session.cpp
    src/recovered_flight.cpp
    src/recovered_ground.cpp
    src/recovered_putter_terminal.cpp
    src/recovered_hazard_recovery.cpp
    src/recovered_distance.cpp
    src/recovered_prng.cpp
    src/recovered_interactions.cpp
    src/recovered_course_lookup.cpp
    src/classic_round_session.cpp
    src/classic_hole_plan.cpp
    src/classic_game_session.cpp
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
add_test(NAME sensigolf_core_contract_tests COMMAND sensigolf_core_contract_tests)

add_executable(sensigolf_recovered_flight_tests
    tests/recovered_flight_tests.cpp
)
target_link_libraries(sensigolf_recovered_flight_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_flight_tests COMMAND sensigolf_recovered_flight_tests)

add_executable(sensigolf_recovered_ground_tests
    tests/recovered_ground_tests.cpp
)
target_link_libraries(sensigolf_recovered_ground_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_ground_tests COMMAND sensigolf_recovered_ground_tests)

add_executable(sensigolf_recovered_probe
    tools/recovered_probe.cpp
)
target_link_libraries(sensigolf_recovered_probe PRIVATE sensigolf_core)

add_executable(sensigolf_recovered_full_probe
    tools/recovered_full_probe.cpp
)
target_link_libraries(sensigolf_recovered_full_probe PRIVATE sensigolf_core)

add_executable(sensigolf_recovered_prng_tests
    tests/recovered_prng_tests.cpp
)
target_link_libraries(sensigolf_recovered_prng_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_prng_tests COMMAND sensigolf_recovered_prng_tests)

add_executable(sensigolf_recovered_prng_probe
    tools/recovered_prng_probe.cpp
)
target_link_libraries(sensigolf_recovered_prng_probe PRIVATE sensigolf_core)

add_executable(sensigolf_recovered_interactions_tests
    tests/recovered_interactions_tests.cpp
)
target_link_libraries(sensigolf_recovered_interactions_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_interactions_tests COMMAND sensigolf_recovered_interactions_tests)

add_executable(sensigolf_recovered_interactions_probe
    tools/recovered_interactions_probe.cpp
)
target_link_libraries(sensigolf_recovered_interactions_probe PRIVATE sensigolf_core)

add_executable(sensigolf_recovered_course_lookup_tests
    tests/recovered_course_lookup_tests.cpp
)
target_link_libraries(sensigolf_recovered_course_lookup_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_course_lookup_tests COMMAND sensigolf_recovered_course_lookup_tests)

add_executable(sensigolf_recovered_course_lookup_probe
    tools/recovered_course_lookup_probe.cpp
)
target_link_libraries(sensigolf_recovered_course_lookup_probe PRIVATE sensigolf_core)


add_executable(sensigolf_classic_shot_model_tests
    tests/classic_shot_model_tests.cpp
)
target_link_libraries(sensigolf_classic_shot_model_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_classic_shot_model_tests COMMAND sensigolf_classic_shot_model_tests)


add_executable(sensigolf_classic_course_resources_tests
    tests/classic_course_resources_tests.cpp
)
target_link_libraries(sensigolf_classic_course_resources_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_classic_course_resources_tests COMMAND sensigolf_classic_course_resources_tests)

add_executable(sensigolf_classic_course_probe
    tools/classic_course_probe.cpp
)
target_link_libraries(sensigolf_classic_course_probe PRIVATE sensigolf_core)


add_executable(sensigolf_classic_terrain_tests
    tests/classic_terrain_tests.cpp
)
target_link_libraries(sensigolf_classic_terrain_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_classic_terrain_tests COMMAND sensigolf_classic_terrain_tests)


add_executable(sensigolf_classic_hole_session_tests
    tests/classic_hole_session_tests.cpp
)
target_link_libraries(sensigolf_classic_hole_session_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_classic_hole_session_tests COMMAND sensigolf_classic_hole_session_tests)


add_executable(sensigolf_recovered_putter_terminal_tests
    tests/recovered_putter_terminal_tests.cpp
)
target_link_libraries(sensigolf_recovered_putter_terminal_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_putter_terminal_tests COMMAND sensigolf_recovered_putter_terminal_tests)

add_executable(sensigolf_recovered_putter_terminal_probe
    tools/recovered_putter_terminal_probe.cpp
)
target_link_libraries(sensigolf_recovered_putter_terminal_probe PRIVATE sensigolf_core)


add_executable(sensigolf_recovered_hazard_recovery_tests
    tests/recovered_hazard_recovery_tests.cpp
)
target_link_libraries(sensigolf_recovered_hazard_recovery_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_hazard_recovery_tests COMMAND sensigolf_recovered_hazard_recovery_tests)

add_executable(sensigolf_recovered_hazard_recovery_probe
    tools/recovered_hazard_recovery_probe.cpp
)
target_link_libraries(sensigolf_recovered_hazard_recovery_probe PRIVATE sensigolf_core)

add_executable(sensigolf_recovered_distance_tests
    tests/recovered_distance_tests.cpp
)
target_link_libraries(sensigolf_recovered_distance_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_recovered_distance_tests COMMAND sensigolf_recovered_distance_tests)


add_executable(sensigolf_classic_round_session_tests
    tests/classic_round_session_tests.cpp
)
target_link_libraries(sensigolf_classic_round_session_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_classic_round_session_tests COMMAND sensigolf_classic_round_session_tests)


add_executable(sensigolf_classic_hole_plan_tests
    tests/classic_hole_plan_tests.cpp
)
target_link_libraries(sensigolf_classic_hole_plan_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_classic_hole_plan_tests COMMAND sensigolf_classic_hole_plan_tests)


add_executable(sensigolf_classic_hole_plan_probe
    tools/classic_hole_plan_probe.cpp
)
target_link_libraries(sensigolf_classic_hole_plan_probe PRIVATE sensigolf_core)


add_executable(sensigolf_classic_game_session_tests
    tests/classic_game_session_tests.cpp
)
target_link_libraries(sensigolf_classic_game_session_tests PRIVATE sensigolf_core)
add_test(NAME sensigolf_classic_game_session_tests COMMAND sensigolf_classic_game_session_tests)


add_executable(sensigolf_classic_game_session_probe
    tools/classic_game_session_probe.cpp
)
target_link_libraries(sensigolf_classic_game_session_probe PRIVATE sensigolf_core)
