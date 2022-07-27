cmake_minimum_required(VERSION 3.5)

project(pcie_sdk_demo)

include_directories(${CMAKE_SOURCE_DIR}/pcie_sdk/include ${CMAKE_SOURCE_DIR}/src)

set(PROJECT_LINK_DIR ${CMAKE_SOURCE_DIR}/pcie_sdk/lib)

if (BUILD_IMAGE_DISP)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif ()

if (WITH_GSTREAM)
    add_definitions(-DWITH_STREAM)
    set(GST_INCLUDE_PATH /usr/local/gstreamer-1.0)
    set(GST_LIBS ${GST_INCLUDE_PATH}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
    include_directories(${GST_INCLUDE_PATH}/include/gstreamer-1.0
                        ${GST_INCLUDE_PATH}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/gstreamer-1.0/include
                        ${GST_INCLUDE_PATH}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/glib-2.0/include/
                        ${GST_INCLUDE_PATH}/include/glib-2.0
    )

    set (GSTREAM_LIBS   ${GST_LIBS}/libgstreamer-1.0.so
                        ${GST_LIBS}/libgstvideo-1.0.so
                        ${GST_LIBS}/libgstapp-1.0.so
                        ${GST_LIBS}/libgstwebrtc-1.0.so
                        ${GST_LIBS}/libgstbase-1.0.so
                        ${GST_LIBS}/libglib-2.0.so
                        ${GST_LIBS}/libgobject-2.0.so
    )

    if (WITH_CUDA)
        add_definitions(-DWITH_CUDA)
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cuda")
        find_package(CUDA)
            message("-- CUDA version: ${CUDA_VERSION}")

        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -O3)
    endif()
endif ()

if (MINGW32)
    link_directories(${PROJECT_LINK_DIR})
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/release/mingw32)

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
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/release/linux)

    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "\${ORIGIN}/../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    if (WITH_GSTREAM)
        add_executable(${PROJECT_NAME}_init src/c/demo_init.c)
        target_link_libraries(${PROJECT_NAME}_init
            ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
            pthread
            anl
            ${GSTREAM_LIBS}
        )
    else ()
        add_executable(${PROJECT_NAME}_init src/c/demo_init.c)
        target_link_libraries(${PROJECT_NAME}_init
            ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
            pthread
            anl
        )
    endif ()

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

#    if (WITH_CUDA)
#        include_directories()
#        add_executable(${PROJECT_NAME}_video_stream src/c/demo_video_stream.cpp
#            src/c/render.cpp
#            )
#        target_link_libraries(${PROJECT_NAME}_video_stream
#            ${OpenCV_LIBS}
#            ${CUDA_LIBRARIES}
#            ${PROJECT_LINK_DIR}/linux/libpcie_sdk.so
#            ${PROJECT_LINK_DIR}/linux/libjetson-utils.so
#            OpenGL::GL
#            OpenGL::GLU
##            glfw
#            ${GSTLIBS_LIBRARIES}
#            ${GLEW_LIBRARIES}
#            pthread
#            anl
#            )
#        set_target_properties(${PROJECT_NAME}_video_stream PROPERTIES LINKER_LANGUAGE CXX)
#        install (TARGETS ${PROJECT_NAME}_video_stream RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
#    endif ()

    FILE(GLOB PCIE_SDK_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/libpcie_sdk.so*")
#    FILE(GLOB PCIE_SDK_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/libjetson-utils.so")
endif()

install (TARGETS ${PROJECT_NAME}_init RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_client RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_service RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (FILES ${PCIE_SDK_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
