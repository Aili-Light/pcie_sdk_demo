ALG PCIE SDK Demo
====================================  

This is demostration codes for developer who work on ALG's PCIE Card utilizations.

For more information check the [website](https://aili-light.com)

# Prerequisites
1. Windows
   * CMake 3.5 or newer
   * Mingw64 gcc version 7.3-posix-seh-rev0
   * Optional : Opencv 3.4.9 (for image display), GStreamer V1.21 (for V4L2/RTP), CUDA Toolkit V11.6 (for Video Codec)  

2. Linux (Ubuntu)
   * CMake 3.5 or newer
   * gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)
   * Optional : Opencv 3.4.9 (for Image Display), GStreamer V1.21 (for V4L2/RTP), CUDA Toolkit V11.6 (for Video Codec)  

# Quick Build Instructions
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DCMAKE_INSTALL_PREFIX=<install path> ..`  
    to build with image display add option : `-DBUILD_IMAGE_DISP=ON`   
    to build with codec add option : `-DWITH_GSTREAMER=ON` and `-DWITH_CUDA=ON`  
4.  `make`  
5.  `make install`  

# Usuage
Init SDK
------------------------------------
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_init -s`   
   
   or use Python :  
   `sudo python init_sdk.py`  
   
Set Sensor Config
------------------------------------
   for all types sensor (use json file)  
   `python set_sensor_from_json.py --json_file=<path to json file> --channel=xx`  

Stream On/Off
------------------------------------
   `cd <src/python>`  
   Stream on
   `python stream_on_by_channel.py --channel='x,y'`  
   
   Stream off
   `python stream_off.py`  

Subscribe to image
------------------------------------
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_client -image <topic_name>`  

Image Display (by OpenCV)  
------------------------------------
   `cd <install path>`  
   display all images :   
   `sudo ./pcie_sdk_demo_image_disp -all`   
   
   display image from select channel :  
   `sudo ./pcie_sdk_demo_image_disp -c <topic_name>`   

Image Display (by OpenGL)  
------------------------------------
   `cd <install path>`  
   display all images :   
   `sudo ./pcie_sdk_demo_gl_disp -all`   

   display image from select channel :  
   `sudo ./pcie_sdk_demo_gl_disp -c <topic_name>` 

Set/Get System Time
------------------------------------
   `cd <src/python>`  
   Set System Time:  
   `python set_time.py --device=xx --time_mode=1 --utc_time=2022:08:23:15:25:30 --unix_time=1660972543000000 --relative_time=0`  
   
   Get System Time:  
   `python get_time.py --device=xx`  

Set Trigger
------------------------------------
   `cd <src/python>`  
   Set Trigger Mode (Int Trigger) :  
   `python set_trigger_mode.py --device=xx --mode=2`  
   
   Set Trigger Mode (Ext Trigger) :  
   `python set_trigger_mode.py --device=xx --mode=1`  
   
   Set Trigger Parameters :  
   `python set_trigger_mode.py --channel=xx,yy --mode=2 --delay_time=0 --valid_time=1000 --polarity=0 --freq=30`  

# Publish ROS Image (Ubuntu)
Build With ROS (melodic / noetic)
------------------------------------
1.   Install ROS melodic.   
2.   After installation, add ROS libraries to PATH:  
     `source /opt/ros/melodic/setup.bash`  
3.   Add option `-DWITH_ROS=ON` in cmake:
     `cmake -DWITH_ROS=ON ..`  
     `make & make install`  

Run ROS Publish
------------------------------------
1.   Start roscore, open Terminal and run :  
     `roscore &`  
2.   Init SDK : 
     `sudo ./pcie_sdk_demo_init -s`  
3.   Set Sensor Config :  
     `python set_sensor_from_json.py --json_file=<path to json file> --channel=xx`  
4.   Stream on :  
     `python stream_on_by_channel.py --channel='x,y'`  
5.   Run Rospub : 
     `sudo ./pcie_sdk_demo_rospub -all`  

Subscribe to ROS Topic 
------------------------------------
ROS Topic Name : `/image_data/stream/xx`  
(xx is channel id starting from 00)  
To display image of channel 0, use image_view :   
`rosrun image_view image_view image:=/image_data/stream/00`  

# Publish ROS2 Image (Ubuntu)
Build With ROS2 (humble)
------------------------------------
1.   Install ROS2 humble.   
2.   After installation, add ROS2 libraries to PATH:  
     `source /opt/ros/humble/setup.bash`  
3.   Add option `-DWITH_ROS2=ON` in cmake:
     `cmake -DWITH_ROS2=ON ..`  
     `make & make install`  

Run ROS2 Publish
------------------------------------ 
1.   Init SDK : 
     `sudo ./pcie_sdk_demo_init -s`  
2.   Set Sensor Config :  
     `python set_sensor_from_json.py --json_file=<path to json file> --channel=xx`  
3.   Stream on :  
     `python stream_on_by_channel.py --channel='x,y'`  
4.   Run Rospub : 
     `sudo ./pcie_sdk_demo_rospub -all`  

Subscribe to ROS2 Topic 
------------------------------------
ROS2 Topic Name : `/image_data/stream/ch-xx`  
(xx is channel id starting from 00)  
To display image of channel 0, use rviz2 :   
`topic=/image_data/stream/ch-00`  

# Support
contact : jimmy@ailiteam.com
