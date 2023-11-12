enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(testmain testmain.cpp)

target_link_libraries(testmain PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(AllTestsInMain testmain)