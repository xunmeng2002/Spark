add_static_md_module(TestCommon ${CMAKE_CURRENT_SOURCE_DIR}/TestCommon ${CMAKE_CURRENT_SOURCE_DIR}/TestCommon)
target_link_libraries(TestCommonStatic PUBLIC Core Network)

add_static_md_module(Packages ${CMAKE_CURRENT_SOURCE_DIR}/Packages ${CMAKE_CURRENT_SOURCE_DIR}/Packages)
target_link_libraries(PackagesStatic PUBLIC Core Network)


add_application(TestMD5         ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/TestMD5)
add_application(TestCore        ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/TestCore)
add_application(TestClient      ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/TestClient)
add_application(TestServer      ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/TestServer)
target_link_libraries(TestMD5 PRIVATE Core)
target_link_libraries(TestCore PRIVATE Core)
target_link_libraries(TestClient PRIVATE TestCommonStatic PackagesStatic)
target_link_libraries(TestServer PRIVATE TestCommonStatic PackagesStatic)


option(BUILD_UNIT_TESTS "Build unit tests" ON)
if(BUILD_UNIT_TESTS)
    add_subdirectory(unittest)
endif()

