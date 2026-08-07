cmake_minimum_required(VERSION 3.20)

find_package(GTest CONFIG REQUIRED)

add_application(UnitTests
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(UnitTests PRIVATE
    GTest::gtest
    GTest::gtest_main
    TemplateLib
    Core
    Serialization
    Network
    TestCommonStatic
    PackagesStatic
)
