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
    to build with v4l2/rtp add option : `-DWITH_GSTREAMER=ON`  
    to build with codec/rtmp add option : `-DWITH_GSTREAMER=ON` and `-DWITH_CUDA=ON`  
4.  `make`  
5.  `make install`  

# GStreamer Support
Install GStreamer (Ubuntu)
------------------------------------
   Download GStrearmer-1.21 (https://gitlab.freedesktop.org/gstreamer/gstreamer.git) and follow the build instruction on README.  
   ** IMPORTANT NOTE : Do not use ubuntu default GStreamer (1.14).  
   ** Version must be >1.20 because a bug fix is merged thereafter :  
   `https://gitlab.freedesktop.org/gstreamer/gstreamer/-/merge_requests/1970`  
   Please specify the prefix path while buding :  
   `meson --prefix=/usr/local/gstreamer-1.0 builddir`  
   After GStreamer has installed, you have to add PATH to your system environment.
1. `vim ~/.bashrc`  
2. append line : `export LD_LIBRARY_PATH=/usr/local/gstreamer-1.0/lib/x86_64-linux-gnu/:$LD_LIBRARY_PATH`  
3. append line : `export GST_PLUGIN_PATH=/usr/local/gstreamer-1.0/lib/x86_64-linux-gnu/gstreamer-1.0/`  
4. append line : `alias sudo='sudo PATH="$PATH" LD_LIBRARY_PATH="$LD_LIBRARY_PATH" GST_PLUGIN_PATH="$GST_PLUGIN_PATH"'`  
5. save .bashrc and then `source ~/.bashrc`  
6. `sudo vim /etc/ld.so.conf`  
7. append line : `/usr/local/gstreamer-1.0/lib/x86_64-linux-gnu/`  
8. `sudo ldconfig`  
   Check if GStreamer is perfectly install. 
   Type : `gst-launch-1.0 --version`  
   You should be able to see :  
    gst-launch-1.0 version 1.21.0  
    GStreamer 1.21.0 (GIT)  
    Unknown package origin  
   
   ** Important : Remember to check gst library paths are correctly configured in sudo envirionment.  
      Type : `sudo su`  
      `echo $LD_LIBRARY_PATH`  
      `echo $GST_PLUGIN_PATH`  
      If the path is not correct (or empty), you should check your sudo environment setting.

   ** If you have problem buiding from source, please contact us for pre-build package.  

Install GStreamer (Windows)
------------------------------------
GStreamer can be built from source on Windows (follow instructions from GStreamer's github page). But for convenience we also provide pre-built package for users. Please contact us for the package.  


# V4L2 Driver  (Ubuntu)
------------------------------------
   Get v4l2loopback source file from Aili-Light (do NOT use soruce from public git repository) and follow the build instruction.
1. `make & sudo make install`  
2. `sudo depmod -a`  
3. `sudo modprobe v4l2loopback`  
   If you see error : Unknown symbol in module, please mannually load videodev kernel:  
   `sudo modprobe videodev`  


# Video Codec With CUDA (Ubuntu)
1. Install nvidia driver :  
   `sudo apt-get install nvidia-driver-xxx`  
   xxx is the suiteble driver version for your GPU (for example 470)  
   After installation finished, use command `nvidia-smi` to check your GPU is correctly recognized.  
2. Install CUDA Toolkit  
   Download cuda : https://developer.nvidia.com/cuda-downloads and follow the instruction.  
   `sudo dpkg -i cuda-repo-xxxx.deb`  
   `sudo apt-key add /var/cuda/repo-xxx`  
   `sudo apt-get update`  
   `sudo apt-get install cuda`  
   After installation, add CUDA to PATH :   
   `export PATH=/usr/local/cuda/bin:$PATH`  
   `export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH`  

3. add options `-DWITH_CUDA=ON` and `-DWITH_GSTREAMER=ON` in cmake  

Important Note : 
------------------------------------
   Nvidia's commercial GPU only allow maximum 3 concurrent video codec sesssions. If you want more than 3 channel video codec, you have to add the patch for CUDA. See the link below :  
   https://github.com/keylase/nvidia-patch/  

# Stream Over RTMP
Install Nginx  (Ubuntu)
------------------------------------
1. Build from source (with rtmp module)  
`git clone https://github.com/arut/nginx-rtmp-module.git`  
`wget http://nginx.org/download/nginx-1.21.5.tar.gz`  
`tar -zvxf nginx-1.21.5.tar.gz`  
`cd nginx-1.21.5`  
`./configure --prefix=/usr/local/nginx  --add-module=../nginx-rtmp-module  --with-http_ssl_module`  
`make -j8`  
`sudo make install`  
2. Open configuration file `/usr/local/nginx/conf/nginx.conf`, add rtmp decription in the end :  
   `rtmp {`  
   `   server {`  
   `      listen 1935;`   
   `      chunk_size 4000;`  
   `      application live {`   
   `            live on;`  
   `      }`  
   `   }`  
   `}`  
3. Start nginx :  `sudo /usr/local/nginx/sbin/nginx`  

Install Nginx  (Windows)
------------------------------------
1. Download pre-built nginx with rtmp module from :  
`http://nginx-win.ecsds.eu/download`  
select `nginx 1.7.11.3 Gryphon.zip`  to download.  
2. Extract zip and open the folder.  
3. Open the file conf/nginx-win.conf, add rtmp decription in the end.
4. Start nginx : `nginx.exe -c conf/nginx-win.conf`  

# ROS Publish (Ubuntu)
Build With ROS (Melodic)
------------------------------------
1.   Install ROS Melodic.   
2.   After installation, add ROS libraries to PATH:  
     `source /opt/ros/melodic/setup.bash`  
3. add option `-DWITH_ROS=ON` in cmake.  

Run ROS Publish
------------------------------------
1.   Start roscore, open Terminal and type :  
     `roscore &`  
2.   Init SDK with ROS : `sudo ./pcie_sdk_demo_init --rospub`  
3.   Set Sensor config and stream on for each channel.  

Subscribe to Image Topic 
------------------------------------
ROS Topic Name : `/image_data/stream/xx`  
(xx is channel id starting from 00)  
To display image, use image_view :   
`rosrun image_view image_view image:=/image_data/stream/00`  

# OpenGL Display Support (with CUDA)
Install GLEW
------------------------------------
1.   `sudo apt-get install libglew-dev glew-utils`  
2.   Install CUDA Toolkit (see above instructions)  
3.   Build SDK with cmake option : `-DBUILD_GL_DISPLAY=ON` and `-DWITH_CUDA=ON`  

# Usuage
Init SDK
------------------------------------
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_init -s`   
   
   or use Python :  
   `sudo python init_sdk.py`  
   
   To use v4l2 video stream, init sdk as :  
   `sudo ./pcie_sdk_demo_init --v4l2loop`   
   
   For video codec (with CUDA), init sdk as :  
   `sudo ./pcie_sdk_demo_init -d` (display only)  
   `sudo ./pcie_sdk_demo_init -r` (both display and save video file)   
   `sudo ./pcie_sdk_demo_init -w` (stream over rtmp)   

Set Sensor Config
------------------------------------
   `cd <src/python>`  
   for ALG sensor  
   `python set_sensor_config.py`   
   
   for non-ALG sensor (use json file)  
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

Image Display
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

Image Display (from V4L2 Device)
------------------------------------
   `cd <install path>`  
   display from v4l2 device :   
   `sudo ./pcie_sdk_demo_v4l2 <device name>`   
   device name example : `/dev/video102`  

   Alternatively, you can use other tools like:  
   Gstreamer (You can change parameters : device, width, height..) :  
   `gst-launch-1.0 v4l2src device=/dev/video102 ! rawvideoparse width=1920 height=1280 format=4 ! videoconvert ! autovideosink`  
    
   Or VLC Player -> Open Capture Device :  
   `/dev/video102`  

Stream Over RTMP
------------------------------------
   To capture video stream :  
   `gst-launch-1.0 rtmp2src location=rtmp://127.0.0.1:1935/live/ch00 ! decodebin ! queue ! videoconvert ! autovideosink sync=false`  
   
   Or Use VLC Player -> Open Network Stream :  
   `rtmp://127.0.0.1:1935/live/ch00`  

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

# Support
contact : jimmy@ailiteam.com
