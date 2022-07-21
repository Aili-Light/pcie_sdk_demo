ALG PCIE SDK Demo
====================================  

This is demostration codes for developer who work on ALG's PCIE Card utilizations.

For more information check the [website](https://aili-light.com)

Prerequisites
------------------------------------
1. Windows
   * CMake 3.5 or newer
   * Mingw64 gcc version 7.3-posix-seh-rev0
   * Optional : opencv 3.4.9 (for image display)

2. Linux (Ubuntu)
   * CMake 3.5 or newer
   * gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)
   * Optional : opencv 3.4.9 with gtk-2.0 (for image display)

V4L2 Video Stream
------------------------------------
# GStreamer
   Download GStrearmer-1.21 (https://gitlab.freedesktop.org/gstreamer/gstreamer.git) and follow the build instruction on README.
   ** IMPORTANT NOTE : Do not use ubuntu default GStreamer (1.14).  
   ** Version must be >1.20 because a bug fix is merged thereafter :  
   `https://gitlab.freedesktop.org/gstreamer/gstreamer/-/merge_requests/1970`  
   Please specify the prefix path while buding :  
   `meson --prefix=/usr/local/gstreamer-1.0 builddir`  
   After GStreamer has installed, you have to add PATH to your system environment.
1. `vim ~/.bashrc`  
2. `export LD_LIBRARY_PATH=/usr/local/gstreamer-1.0/lib/x86_64-linux-gnu/$LD_LIBRARY_PATH`  
3. `export GST_PLUGIN_PATH=/usr/local/gstreamer-1.0/lib/x86_64-linux-gnu/gstreamer-1.0/`  
4. `alias sudo='sudo PATH="$PATH" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" GST_PLUGIN_PATH="$GST_PLUGIN_PATH"'`  
5. `source ~/.bashrc`  
6. `sudo vim /etc/ld.so.conf`  
7. `/usr/local/gstreamer-1.0/lib/x86_64-linux-gnu/`  
8. `sudo ldconfig`  
   Check if GStreamer is perfectly install. 
   Type `gst-launch-1.0 --version`  
   You should be able to see :  
    gst-launch-1.0 version 1.21.0  
    GStreamer 1.21.0 (GIT)  

# virtual v4l2 device
   Download v4l2loopback from source (https://github.com/umlaeute/v4l2loopback) and follow the build instruction.
1. `make & sudo make install`  
2. `sudo depmod -a`  
3. `sudo modprobe v4l2loopback`  
4. `sudo su`  
5. `echo "v4l2loopback" | tee /etc/modules-load.d/modules.conf`  
6. `echo "options v4l2loopback video_nr=0,1,2,3,4,5,6,7 card_label=\"gmsl camera\" exclusive_caps=1" | tee /etc/modprobe.d/v4l2loopback.conf`  
7. `update-initramfs -c -k $(uname -r)`  
8. `reboot`  
   After reboot, you should be able to see multiple virtual v4l2 devices by typing :
   `ls /dev/video*`  

Quick Build Instructions
------------------------------------
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DCMAKE_INSTALL_PREFIX=<install path> ..`  
    to build image display add option : `-DBUILD_IMAGE_DISP=ON`   
    to build GStreamer (rtp/v4l2) add option : `-DWITH_GSTREAM=ON`  
4.  `make`  
5.  `make install`  

Usuage
------------------------------------
# Init SDK
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_init -s`   
   or use  
   `sudo python init_sdk.py`  
   To use v4l2 video stream, init sdk as :  
   `sudo ./pcie_sdk_demo_init -v`   

# Set Sensor Config
   `cd <src/python>`  
   for ALG sensor  
   `python set_sensor_config.py`   
   for non-ALG sensor (require config table)  
   `python set_sensor_config_from_file.py`  
   or use json file
   `python set_sensor_from_json.py --json_file=<path to json file> --channel=xx`  

# Stream On/Off
   `cd <src/python>`  
   Stream on
   `python stream_on.py`  
   or use
   `python stream_on_by_channel.py --channel='x,y'`  
   Stream off
   `python stream_off.py`  

# Subscribe to image
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_client -image <topic_name>`  

# Image Display
   `cd <install path>`  
   display all images :   
   `sudo ./pcie_sdk_demo_image_disp -all`   
   display image from select channel :   
   `sudo ./pcie_sdk_demo_image_disp -c <topic_name>`   

# V4L2 Video Stream
   To capture video stream :  
   `gst-launch-1.0 v4l2src device=/dev/video0 ! rawvideoparse width=1920 height=1280 format=4 ! videoconvert ! autovideosink`  
   (You may want to change the parameters : device#, with, height,.. for your case)  

Support
------------------------------------
contact : jimmy@ailiteam.com