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
# Windows
------------------------------------
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DCMAKE_INSTALL_PREFIX=<install path> ..`  
4.  `make`  
5.  `make install`  

# Linux (Ubuntu)
------------------------------------
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DCMAKE_INSTALL_PREFIX=<install path> ..`
    to build image display add option `-DBUILD_IMAGE_DISP=ON`    
4.  `make`  
5.  `make install`  

Usuage
------------------------------------
# Init SDK
   `cd instal`  
   `sudo ./pcie_sdk_demo_init -s`   

# Subscribe
   `cd install`  
   `sudo ./pcie_sdk_demo_client -c <topic_name>`  

# Image Display
   `cd install`  
   `sudo ./pcie_sdk_demo_image_disp -c /image_data/stream/xx`   

Support
------------------------------------
contact : jimmy@ailiteam.com


