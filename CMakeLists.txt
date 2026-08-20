cmake_minimum_required(VERSION 3.20)
include(submodules/CMakeCommon/CMakeCommon.cmake)
restore_vcpkg_root()
reuse_git_proxy_for_vcpkg()
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
message(STATUS "ENV{VCPKG_ROOT} = $ENV{VCPKG_ROOT}")
message(STATUS "ENV{VCPKG_DOWNLOADS} = $ENV{VCPKG_DOWNLOADS}")
message(STATUS "ENV{VCPKG_DEFAULT_BINARY_CACHE} = $ENV{VCPKG_DEFAULT_BINARY_CACHE}")
message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
SET(PROJECT_NAME "Spark")
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)
include(GenerateExportHeader)
include(submodules/CMakeCommon/CMakeCommon.cmake)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_VERBOSE_MAKEFILE OFF)
add_definitions(-DUNICODE -D_UNICODE)


IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /bigobj /utf-8")
    add_definitions(-DWINDOWS -D_CRT_SECURE_NO_WARNINGS)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	add_compile_options(
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/Zi>
        $<$<CONFIG:MinSizeRel>:/O1>)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions("-DLINUX")
    add_compile_options(
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-ggdb>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:RelWithDebInfo>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-g>
        $<$<CONFIG:MinSizeRel>:-Os>)
    add_compile_definitions(
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:RelWithDebInfo>:NDEBUG>
        $<$<CONFIG:MinSizeRel>:NDEBUG>)
    #list(APPEND CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address")
ENDIF()
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
