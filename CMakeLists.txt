cmake_minimum_required(VERSION 3.5)

project(pcie_sdk_demo)
set(CMAKE_CXX_STANDARD 11)

message(STATUS "Processor name is ${CMAKE_SYSTEM_PROCESSOR}")
include_directories(${CMAKE_SOURCE_DIR}/pcie_sdk/include ${CMAKE_SOURCE_DIR}/src/c)
set(PROJECT_LINK_DIR ${CMAKE_SOURCE_DIR}/pcie_sdk/lib)

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
    
    add_executable(${PROJECT_NAME}_init src/c/demo/demo_init.c)
    target_link_libraries(${PROJECT_NAME}_init
        ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
        pthread
        anl
    )

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

    if (WITH_ROS)
        message("-- Build with ROS")
        add_definitions(-DWITH_ROS)
        find_package(catkin REQUIRED COMPONENTS
            roscpp
            rospy
            std_msgs
            sensor_msgs
        )
        include_directories(
            ${catkin_INCLUDE_DIRS}
        )
        aux_source_directory(src/c/alg_rosbridge ROS_BRIDGE_SRC)
        add_executable(${PROJECT_NAME}_rospub
            src/c/demo/demo_rospub.cpp
            ${ROS_BRIDGE_SRC}
        )
        target_link_libraries(${PROJECT_NAME}_rospub
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_cvtColor.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_IspPipe.so    
            pthread
            anl
            ${catkin_LIBRARIES}
        )
        set_target_properties(${PROJECT_NAME}_rospub PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_rospub RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
        FILE(GLOB ALG_CVT_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv*")
        install (FILES ${ALG_CVT_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
    endif ()

    if (WITH_ROS2)
        message("-- Build with ROS2")
        add_definitions(-DWITH_ROS2)
        find_package(ament_cmake REQUIRED)
        find_package(rclcpp REQUIRED)
        find_package(rcl_interfaces REQUIRED)
        find_package(sensor_msgs REQUIRED)
        
        include_directories(
            ${rclcpp_INCLUDE_DIRS}
            ${rcl_interfaces_INCLUDE_DIRS}
            ${sensor_msgs_INCLUDE_DIRS}
        )

        aux_source_directory(src/c/alg_ros2bridge ROS2_BRIDGE_SRC)
        add_executable(${PROJECT_NAME}_ros2pub
            src/c/demo/demo_rospub.cpp
            ${ROS2_BRIDGE_SRC}
        )
        target_link_libraries(${PROJECT_NAME}_ros2pub
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_cvtColor.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_IspPipe.so    
            pthread
            anl
            ${ament_LIBRARIES}
            ${rclcpp_LIBRARIES}
            ${sensor_msgs_LIBRARIES}
        )
        set_target_properties(${PROJECT_NAME}_ros2pub PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_ros2pub RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
        FILE(GLOB ALG_CVT_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv*")
        install (FILES ${ALG_CVT_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
    endif ()

    if (BUILD_IMAGE_DISP)
        find_package(OpenCV REQUIRED)
        include_directories(${OpenCV_INCLUDE_DIRS})
        add_executable(${PROJECT_NAME}_image_disp
            src/c/demo/demo_image_display.cpp
        )
        target_link_libraries(${PROJECT_NAME}_image_disp
            ${OpenCV_LIBS}
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libpcie_sdk.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_cvtColor.so
            ${PROJECT_LINK_DIR}/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv_IspPipe.so            
            pthread
            anl
        )
        if(OpenMP_CXX_FOUND)
            add_definitions(-DWITH_OPENMP)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
            target_link_libraries(${PROJECT_NAME}_image_disp OpenMP::OpenMP_CXX)
        endif()
        set_target_properties(${PROJECT_NAME}_image_disp PROPERTIES LINKER_LANGUAGE CXX)
        install (TARGETS ${PROJECT_NAME}_image_disp RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
        FILE(GLOB ALG_CVT_LIB "${CMAKE_SOURCE_DIR}/pcie_sdk/lib/linux/${CMAKE_SYSTEM_PROCESSOR}/libalg_cv*")
        install (FILES ${ALG_CVT_LIB} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
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
