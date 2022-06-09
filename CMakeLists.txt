cmake_minimum_required(VERSION 3.5)

project(pcie_sdk LANGUAGES C)

include_directories(
    include
)
set(PROJECT_LINK_DIR ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/install)

if (MINGW32)
    link_directories(${PROJECT_LINK_DIR})

    add_executable(${PROJECT_NAME}_demo_init src/c/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_demo_init     
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_demo_client src/c/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_demo_client     
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_demo_service src/c/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_demo_service     
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )
else()
    set(CMAKE_BUILD_RPATH ${PROJECT_LINK_DIR}/linux)
    set(CMAKE_INSTALL_RPATH ${PROJECT_LINK_DIR}/linux)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    add_executable(${PROJECT_NAME}_demo_init src/c/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_demo_init     
        ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
        pthread
        anl
    )

    add_executable(${PROJECT_NAME}_demo_client src/c/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_demo_client     
        ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
        pthread
        anl
    )

    add_executable(${PROJECT_NAME}_demo_service src/c/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_demo_service     
        ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
        pthread
        anl
    )

endif()

install (TARGETS ${PROJECT_NAME}_demo_init RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_demo_client RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_demo_service RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
