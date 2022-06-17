cmake_minimum_required(VERSION 3.5)

project(pcie_sdk)

include_directories(${CMAKE_SOURCE_DIR}/pcie_sdk/include)

set(PROJECT_LINK_DIR ${CMAKE_SOURCE_DIR}/pcie_sdk/lib)

if (BUILD_IMAGE_DISP)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif ()

if (MINGW32)
    link_directories(${PROJECT_LINK_DIR})
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/install/mingw32)

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
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/install/linux)

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
    if (BUILD_IMAGE_DISP)
        add_executable(${PROJECT_NAME}_demo_image_disp src/c/demo_image_display.cpp)
        target_link_libraries(${PROJECT_NAME}_demo_image_disp
            ${OpenCV_LIBS}     
            ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
            pthread
            anl
        )
        set_target_properties(${PROJECT_NAME}_demo_image_disp PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_demo_image_disp RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif ()
endif()

install (TARGETS ${PROJECT_NAME}_demo_init RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_demo_client RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_demo_service RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
