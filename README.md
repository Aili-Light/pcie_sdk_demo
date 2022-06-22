ALG PCIE SDK Demo
====================================  

This is demostration codes for developer who work on ALG's PCIE Card utilizations.

For more information check the [website](https://aili-light.com)

Prerequisites
------------------------------------

1. Windows
   * CMake 3.5 or newer
   * Mingw gcc version 7.3-win32 20180312 (GCC)

2. Linux (Ubuntu)
   * CMake 3.5 or newer
   * gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)
   * opencv 3.4.9 with gtk-2.0 (for image display)

Quick Build Instructions
------------------------------------
# Windows
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DMINGW32=ON -DCMAKE_INSTALL_PREFIX=<install path> ..`  
4.  `make`  
5.  `make install`  

# Linux (Ubuntu)
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DCMAKE_INSTALL_PREFIX=<install path> ..`  
    to build image display add option : `-DBUILD_IMAGE_DISP=ON`    
4.  `make`  
5.  `make install`  

Usuage
------------------------------------
# Init SDK
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_init -s`   

# Set Sensor Config
   `cd <src/python>`  
   set ALG sensor  
   `python3 set_sensor_config.py`   
   set non-ALG sensor (require config file)  
   `python3 set_sensor_config_from_file.py`  

# Stream On/Off
   `cd <src/python>`  
   `python3 stream_on.py`  
   `python3 stream_off.py`  

# Subscribe to image
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_client -image <topic_name>`  

# Image Display
   `cd <install path>`  
   display all images :   
   `sudo ./pcie_sdk_demo_image_disp -all`   
   display image from 1 channel :   
   `sudo ./pcie_sdk_demo_image_disp -c <topic_name>`   

Support
------------------------------------
contact : jimmy@ailiteam.com
