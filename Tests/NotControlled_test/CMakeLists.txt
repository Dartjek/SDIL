set(TEST_NAME NotControlled_LifeTime)
add_executable(${TEST_NAME} main.cpp)
target_link_libraries(${TEST_NAME} SDIL)

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})