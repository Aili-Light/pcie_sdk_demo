cmake_minimum_required(VERSION 3.5)

project(pcie_sdk LANGUAGES C)

include_directories(
    include
)

if (MINGW32)
    set(PROJECT_LINK_DIR ${PROJECT_SOURCE_DIR}/lib/mingw32)
    add_executable(${PROJECT_NAME}_demo_init src/c/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_demo_init     
        ${PROJECT_LINK_DIR}/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_demo_client src/c/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_demo_client     
        ${PROJECT_LINK_DIR}/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_demo_service src/c/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_demo_service     
        ${PROJECT_LINK_DIR}/libpcie_sdk.dll
        pthread
    )
else()
    set(PROJECT_LINK_DIR ${PROJECT_SOURCE_DIR}/lib/linux)
    add_executable(${PROJECT_NAME}_demo_init src/c/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_demo_init     
        ${PROJECT_LINK_DIR}/libpcie_sdk.so
        pthread
    )

    add_executable(${PROJECT_NAME}_demo_client src/c/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_demo_client     
        ${PROJECT_LINK_DIR}/libpcie_sdk.so
        pthread
    )

    add_executable(${PROJECT_NAME}_demo_service src/c/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_demo_service     
        ${PROJECT_LINK_DIR}/libpcie_sdk.so
        pthread
    )

endif()
