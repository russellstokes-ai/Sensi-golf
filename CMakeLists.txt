cmake_minimum_required(VERSION 3.16)
project(sensi_golf_enhanced LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()
add_subdirectory(engine)

if(NOT ANDROID)
    add_executable(pixtee_native_green_arrow_tests
        pixtee-godot/native/tests/native_green_arrow_mapi_bridge_tests.cpp
        pixtee-godot/native/src/pixtee_round_runtime.cpp
        pixtee-godot/native/src/pixtee_course_adapter.cpp
        pixtee-godot/native/src/recovered_welly_tick.cpp)
    target_include_directories(pixtee_native_green_arrow_tests PRIVATE pixtee-godot/native/src)
    target_link_libraries(pixtee_native_green_arrow_tests PRIVATE sensigolf_core)
    add_test(NAME pixtee_native_green_arrow_tests COMMAND pixtee_native_green_arrow_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/pixtee-godot/courses/lakewood-hole-1/native-green-v1")
    add_executable(pixtee_authored_tree_contacts_tests engine/tests/authored_tree_contacts_tests.cpp)
    target_link_libraries(pixtee_authored_tree_contacts_tests PRIVATE sensigolf_core)
    add_test(NAME pixtee_authored_tree_contacts_tests COMMAND pixtee_authored_tree_contacts_tests)
    add_executable(pixtee_approved_lakewood_lookup_tests
        pixtee-godot/native/tests/approved_lakewood_lookup_tests.cpp)
    target_link_libraries(pixtee_approved_lakewood_lookup_tests PRIVATE sensigolf_core)
    add_test(NAME pixtee_approved_lakewood_lookup_tests
        COMMAND pixtee_approved_lakewood_lookup_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/pixtee-godot/courses/lakewood-hole-1"
        "${CMAKE_CURRENT_SOURCE_DIR}/pixtee-godot/courses/lakewood-hole-1/native-green-v1")
endif()
