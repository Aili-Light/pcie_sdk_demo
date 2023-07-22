cmake_minimum_required(VERSION 3.5)

project(pcie_sdk_demo)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    add_definitions(-DWITH_OPENMP)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

message(STATUS "Processor name is ${CMAKE_SYSTEM_PROCESSOR}")
include_directories(${CMAKE_SOURCE_DIR}/pcie_sdk/include ${CMAKE_SOURCE_DIR}/src/c)
set(PROJECT_LINK_DIR ${CMAKE_SOURCE_DIR}/pcie_sdk/lib)

if (WITH_GSTREAMER)
    if (MINGW32)
        include_directories(stream)
        if (WITH_CUDA)
            add_definitions(-DWITH_CUDA)
            include_directories(${CUDA_INCLUDE_DIRS})
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cuda")
            find_package(CUDA)
                message("-- CUDA version: ${CUDA_VERSION}")

            set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -O3)
        endif()
    else ()
        add_definitions(-DWITH_STREAM)
        include_directories(stream/include)

        find_package(PkgConfig REQUIRED)
        pkg_search_module(GST_LIBS REQUIRED gstreamer-1.0 gobject-2.0 glib-2.0)
        MESSAGE(STATUS "GST_LIBS_INCLUDE_DIRS:   ${GST_LIBS_INCLUDE_DIRS}")
        include_directories(${GST_LIBS_INCLUDE_DIRS})
        
        if (WITH_CUDA)
            add_definitions(-DWITH_CUDA)
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cuda")
            find_package(CUDA)
                message("-- CUDA version: ${CUDA_VERSION}")

            set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -O3)
        endif()
    endif ()
endif ()

if (MINGW32)
    link_directories(${PROJECT_LINK_DIR})
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/release/mingw32)

    add_executable(${PROJECT_NAME}_init src/c/demo/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_init
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_client src/c/demo/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_client
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )

    add_executable(${PROJECT_NAME}_service src/c/demo/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_service
        ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
        pthread
    )

    if (WITH_GSTREAMER)
        if (WITH_CUDA)
            include_directories(${CUDA_INCLUDE_DIRS})
        endif()
        add_executable(${PROJECT_NAME}_gst_camera 
            src/c/demo/demo_gst_camera.cpp
            src/c/alg_camera/gst_camera.cpp
        )
        target_link_libraries(${PROJECT_NAME}_gst_camera
            ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk_stream.dll
            ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk_cuda.dll
            pthread
        )
        set_target_properties(${PROJECT_NAME}_gst_camera PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_gst_camera RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif ()

    if (BUILD_IMAGE_DISP)
        find_package(OpenCV REQUIRED)
        include_directories(${OpenCV_INCLUDE_DIRS})
        add_executable(${PROJECT_NAME}_image_disp
            src/c/demo/demo_image_display.cpp
            src/c/alg_cvt/alg_cvtColor.cpp
            )
        target_link_libraries(${PROJECT_NAME}_image_disp
            ${OpenCV_LIBS}
            ${PROJECT_LINK_DIR}/mingw32/libpcie_sdk.dll
            pthread
        )
        set_target_properties(${PROJECT_NAME}_image_disp PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_image_disp RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif ()
    FILE(GLOB PCIE_SDK_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/mingw32/libpcie_sdk*")
    install (FILES ${PCIE_SDK_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/bin)
else()
    set(CMAKE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/release/linux)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "\${ORIGIN}/../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    
    if (WITH_ROS)
        message("-- Build with ROS")
        add_executable(${PROJECT_NAME}_init src/c/demo/demo_init.c)
        add_dependencies(${PROJECT_NAME}_init 
                        ${${PROJECT_NAME}_EXPORTED_TARGETS} 
                        ${catkin_EXPORTED_TARGETS}
        )
        target_link_libraries(${PROJECT_NAME}_init
            ${PROJECT_LINK_DIR}/linux/libpcie_sdk_ros.so
            pthread
            anl
            ${catkin_LIBRARIES}
        )
    else ()
        add_executable(${PROJECT_NAME}_init src/c/demo/demo_init.c)
        target_link_libraries(${PROJECT_NAME}_init
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
            pthread
            anl
        )
    endif ()

    add_executable(${PROJECT_NAME}_client src/c/demo/demo_client.c)
    target_link_libraries(${PROJECT_NAME}_client
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
        pthread
        anl
    )

    add_executable(${PROJECT_NAME}_service src/c/demo/demo_service.c)
    target_link_libraries(${PROJECT_NAME}_service
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
        pthread
        anl
    )

    if (WITH_GSTREAMER)
        add_executable(${PROJECT_NAME}_gst_camera 
                    src/c/demo/demo_gst_camera.cpp
                    src/c/alg_camera/gst_camera.cpp
        )
        target_link_libraries(${PROJECT_NAME}_gst_camera
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk_stream.so
            pthread
            anl
        )
        set_target_properties(${PROJECT_NAME}_gst_camera PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_gst_camera RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif ()

    if (BUILD_IMAGE_DISP)
        find_package(OpenCV REQUIRED)
        include_directories(${OpenCV_INCLUDE_DIRS})
        add_executable(${PROJECT_NAME}_image_disp
            src/c/demo/demo_image_display.cpp
            src/c/alg_cvt/alg_cvtColor.cpp
            )
        target_link_libraries(${PROJECT_NAME}_image_disp
            ${OpenCV_LIBS}
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
            pthread
            anl
        )
        if(OpenMP_CXX_FOUND)
            target_link_libraries(${PROJECT_NAME}_image_disp
                OpenMP::OpenMP_CXX
            )
        endif()
        set_target_properties(${PROJECT_NAME}_image_disp PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_image_disp RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif ()

    if (BUILD_GL_DISPLAY)
        find_package(CUDA REQUIRED)
        find_package(OpenGL REQUIRED)
        find_package(GLEW REQUIRED)
        add_executable(${PROJECT_NAME}_gl_disp
            src/c/demo/demo_opengl_display.cpp
            src/c/alg_camera/alg_camera.cpp
        )
        target_link_libraries(${PROJECT_NAME}_gl_disp
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk_cuda.so
            pthread
            anl
            X11
            OpenGL::GL
            GLEW::GLEW
        )
        set_target_properties(${PROJECT_NAME}_gl_disp PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_gl_disp RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)

        add_executable(${PROJECT_NAME}_v4l2
            src/c/demo/demo_v4l2.cpp
            src/c/alg_camera/alg_camera.cpp
            src/c/alg_camera/v4l2_camera.cpp
        )
        target_link_libraries(${PROJECT_NAME}_v4l2
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk_cuda.so
            pthread
            anl
            X11
            OpenGL::GL
            GLEW::GLEW
        )
        set_target_properties(${PROJECT_NAME}_v4l2 PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_v4l2 RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
    endif()

    FILE(GLOB PCIE_SDK_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk*")
    install (FILES ${PCIE_SDK_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
endif()

install (TARGETS ${PROJECT_NAME}_init RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_client RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install (TARGETS ${PROJECT_NAME}_service RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
