cmake_minimum_required(VERSION 3.20)

add_application(UnitTests
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# doctest 默认自带 std 前向声明；本仓测试在 doctest 之后才 include std 头，须改用真实头避免类型不完整
target_compile_definitions(UnitTests PRIVATE DOCTEST_CONFIG_USE_STD_HEADERS)

target_link_libraries(UnitTests PRIVATE
    doctest::doctest
    TemplateLib
    Core
    Serialization
    Network
    TestCommonStatic
    PackagesStatic
)
