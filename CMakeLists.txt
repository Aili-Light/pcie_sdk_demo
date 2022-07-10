cmake_minimum_required(VERSION 3.5)

project(pcie_sdk_demo)

include_directories(${CMAKE_SOURCE_DIR}/pcie_sdk/include ${CMAKE_SOURCE_DIR}/src)

set(PROJECT_LINK_DIR ${CMAKE_SOURCE_DIR}/pcie_sdk/lib)

if (BUILD_IMAGE_DISP)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif ()

if (BUILD_WITH_CUDA)
    set(CMAKE_CXX_FLAGS "-std=c++11")
    find_package(CUDA REQUIRED)
    if (CUDA_FOUND)
        include_directories(${CUDA_INCLUDE_DIRS})
        message("-- CUDA version: ${CUDA_VERSION}")
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -O3)
    endif ()

    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    if (GLEW_FOUND)
        include_directories(%{GLEW_INCLUDE_DIRS})
        message("-- Found GLEW")
    endif ()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GSTLIBS REQUIRED
                    gstreamer-pbutils-1.0
                    gstreamer-video-1.0
                    gstreamer-app-1.0
                    gstreamer-sdp-1.0
                    gstreamer-webrtc-1.0
                    gstreamer-1.0

    )
    include_directories(/usr/include/gstreamer-1.0
                        /usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/gstreamer-1.0/include
                        /usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/glib-2.0/include/
                        /usr/include/glib-2.0
    )
endif ()

if (MINGW32)
    link_directories(${PROJECT_LINK_DIR})
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/install/mingw32)

#    set(CMAKE_SKIP_BUILD_RPATH FALSE)
#    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#    set(CMAKE_INSTALL_RPATH "\${ORIGIN}/../lib")
#    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    add_executable(${PROJECT_NAME}_init src/c/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_init
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_client src/c/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_client
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_service src/c/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_service
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )
    if (BUILD_IMAGE_DISP)
        add_executable(${PROJECT_NAME}_image_disp src/c/demo_image_display.cpp)
        target_link_libraries(${PROJECT_NAME}_image_disp
            ${OpenCV_LIBS}
            ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
            pthread
        )
        set_target_properties(${PROJECT_NAME}_image_disp PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_image_disp RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif ()
    FILE(GLOB PCIE_SDK_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/mingw32/libpcie_sdk.dll")
else()
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/install/linux)

    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "\${ORIGIN}/../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    add_executable(${PROJECT_NAME}_init src/c/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_init
        ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
        pthread
        anl
    )

    add_executable(${PROJECT_NAME}_client src/c/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_client
        ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
        pthread
        anl
    )

    add_executable(${PROJECT_NAME}_service src/c/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_service
        ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
        pthread
        anl
    )
    if (BUILD_IMAGE_DISP)
        if (BUILD_WITH_CUDA)
            include_directories()
            add_executable(${PROJECT_NAME}_image_disp_cuda src/c/demo_image_display_cuda.cpp src/c/host_memory_manager.cpp)
            target_link_libraries(${PROJECT_NAME}_image_disp_cuda
                ${OpenCV_LIBS}
                ${CUDA_LIBRARIES}
                ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
                ${PROJECT_LINK_DIR}/linux/libjetson-utils.so
                OpenGL::GL
                OpenGL::GLU
                glfw
                ${GSTLIBS_LIBRARIES}
                ${GLEW_LIBRARIES}
                pthread
                anl
            )
            set_target_properties(${PROJECT_NAME}_image_disp_cuda PROPERTIES LINKER_LANGUAGE CXX)
            install (TARGETS ${PROJECT_NAME}_image_disp_cuda RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
        else ()
            add_executable(${PROJECT_NAME}_image_disp src/c/demo_image_display.cpp)
            target_link_libraries(${PROJECT_NAME}_image_disp
                ${OpenCV_LIBS}
                ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
                pthread
                anl
            )
            set_target_properties(${PROJECT_NAME}_image_disp PROPERTIES LINKER_LANGUAGE CXX)
            install (TARGETS ${PROJECT_NAME}_image_disp RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
        endif ()
    endif ()

    FILE(GLOB PCIE_SDK_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/libpcie_sdk.so*")
    FILE(GLOB PCIE_SDK_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/libjetson-utils.so")
endif()

install (TARGETS ${PROJECT_NAME}_init RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_client RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_service RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (FILES ${PCIE_SDK_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
