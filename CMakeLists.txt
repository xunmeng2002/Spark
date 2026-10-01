cmake_minimum_required(VERSION 3.25)
include(submodules/CMakeCommon/CMakeCommon.cmake)
restore_vcpkg_root()
reuse_git_proxy_for_vcpkg()
# VCPKG_ROOT 在 Windows 注册表中是反斜杠形式（如 D:\Github\vcpkg），直接拼接成工具链路径
# 会让 CMake 写进 CMakeSystem.cmake 的 include() 里把 \G 当成非法转义，故先规范化为正斜杠
file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" SPARK_VCPKG_ROOT)
set(CMAKE_TOOLCHAIN_FILE "${SPARK_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
message(STATUS "ENV{VCPKG_ROOT} = $ENV{VCPKG_ROOT}")
message(STATUS "ENV{VCPKG_DOWNLOADS} = $ENV{VCPKG_DOWNLOADS}")
message(STATUS "ENV{VCPKG_DEFAULT_BINARY_CACHE} = $ENV{VCPKG_DEFAULT_BINARY_CACHE}")
message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
SET(PROJECT_NAME "Spark")
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)
include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_VERBOSE_MAKEFILE OFF)
# 平台/配置相关编译选项：MSVC 的 /EHsc、CMAKE_MSVC_RUNTIME_LIBRARY 默认值及各配置优化级别、
# Linux 的 -g/-O 系、NDEBUG/_DEBUG 均为 CMake+编译器默认行为，不再显式重复；仅保留净增量项
if(WIN32)
    add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
    # 警告级别须显式指定：CMP0092 在本项目（cmake_minimum_required 3.25）下为 NEW，
    # CMake 不再向 CMAKE_CXX_FLAGS 注入默认级别，cl.exe 会退回自身默认级别（低于 CMake 旧默认），
    # 等于本项目从未选过级别。写死在此，不随 CMake 版本漂移。
    add_compile_options(/W3 /utf-8 /bigobj)
endif()
# §4「非静态数据成员声明顺序决定初始化顺序，初始化列表需一致」由编译器警告兜住。
# GCC/Clang 的 -Wreorder 不在默认告警集内，需显式打开；MSVC 的对应项是 C5038，
# 该警告属 /W4 级，本项目未开 /W4，故 Windows 侧暂不启用。
if(NOT MSVC)
    add_compile_options(-Wreorder)
endif()
#list(APPEND CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address")
message("CMAKE_BUILD_TYPE:" ${CMAKE_BUILD_TYPE})
message("CMAKE_SYSTEM_NAME:" ${CMAKE_SYSTEM_NAME})
message("CMAKE_CL_64:" ${CMAKE_CL_64})
message("CMAKE_CXX_FLAGS:" ${CMAKE_CXX_FLAGS})
message("CMAKE_CXX_FLAGS_RELEASE:" ${CMAKE_CXX_FLAGS_RELEASE})
message("CMAKE_CXX_FLAGS_DEBUG:" ${CMAKE_CXX_FLAGS_DEBUG})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/$<CONFIG>)




add_template_library(TemplateLib    ${CMAKE_CURRENT_SOURCE_DIR}/include)
#add_static_module(Core              ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/Core)
#add_static_module(Network           ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/Network)
#add_static_module(Serialization     ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization)
#target_link_libraries(NetworkStatic PUBLIC CoreStatic)

add_shared_module(Core				${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/Core)
add_shared_module(Network			${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/Network)
add_shared_module(Serialization	    ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization)
# 导出类的私有 STL 成员触发 MSVC C4251（成员类型自身无 dll 接口）。本仓三个 DLL 与其全部
# 消费者（含测试）出自同一次构建、同一工具集，_ITERATOR_DEBUG_LEVEL 与 MSVC_RUNTIME_LIBRARY
# 不可能分叉，故在该边界上豁免。必须 PUBLIC：告警报在消费者的 TU 上（客户端编译到导出类的
# 内联成员时会实例化该 STL 成员），PRIVATE 盖不到。失效条件＝出现第三方消费者，或消费者与
# 本仓工具集版本分叉——届时须重评（pimpl 或另做接口层），不得继续沿用本豁免。
if(MSVC)
    target_compile_options(Core PUBLIC /wd4251)
    target_compile_options(Network PUBLIC /wd4251)
    target_compile_options(Serialization PUBLIC /wd4251)
endif()
target_compile_definitions(Serialization PRIVATE JSON_DLL_BUILD)
# Encode.cpp 使用了 C++17 弃用的 <codecvt>，仅对该文件压制警告
if(LINUX)
	set_source_files_properties(
		${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization/Encode/Encode.cpp
		PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations"
	)
endif()
if(WIN32)
	set_source_files_properties(
		${CMAKE_CURRENT_SOURCE_DIR}/src/Serialization/Encode/Encode.cpp
		PROPERTIES COMPILE_FLAGS "-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING"
	)
endif()
target_link_libraries(Network PUBLIC Core)


option(BUILD_TESTS "Build test projects" ON)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()

string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d-%H%M%S")
message("Build timestamp is ${BUILD_TIMESTAMP}")

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS
    TemplateLib
    Core Network Serialization
    #CoreStatic NetworkStatic SerializationStatic
    EXPORT ${PROJECT_NAME}Targets
    #CONFIGURATIONS Release
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    "include(CMakeFindDependencyMacro)\n"
    "include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake\")\n"
    "set(${PROJECT_NAME}_FOUND TRUE)\n"
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
