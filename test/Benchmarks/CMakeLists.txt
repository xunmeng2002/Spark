# 可执行文件统一用 Bench 前缀，与 Test* 一样在 bin/<Config> 里排在一处；
# 每个基准独占一个同名源目录（add_application 会递归收集源目录下的全部 .cpp），
# 计时器是公开头 <Spark/TemplateLib/Benchmark/BenchmarkRunner.h>，兄弟仓可直接复用
add_application(BenchTimeUtility    ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/BenchTimeUtility)
add_application(BenchLogger         ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/BenchLogger)

target_link_libraries(BenchTimeUtility PRIVATE Core)
target_link_libraries(BenchLogger PRIVATE Core)
