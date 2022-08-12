ALG PCIE SDK Demo
====================================  

This is demostration codes for developer who work on ALG's PCIE Card utilizations.

For more information check the [website](https://aili-light.com)

# Prerequisites
1. Windows
   * CMake 3.5 or newer
   * Mingw64 gcc version 7.3-posix-seh-rev0
   * Optional : opencv 3.4.9 (for image display)

2. Linux (Ubuntu)
   * CMake 3.5 or newer
   * gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)
   * Optional : opencv 3.4.9 with gtk-2.0 (for image display)

# Quick Build Instructions
1.  `mkdir build`  
2.  `cd build`  
3.  `cmake -DCMAKE_INSTALL_PREFIX=<install path> ..`  
    to build with image display add option : `-DBUILD_IMAGE_DISP=ON`   
    to build with v4l2/rtp add option : `-DWITH_GSTREAM=ON`  
    to build with codec/rtmp add option : `-DWITH_GSTREAM=ON` and `-DWITH_CUDA=ON`  
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


# Virtual v4l2 device  (Ubuntu)
------------------------------------
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
   `/dev/video0` `/dev/video1`  ....  

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

3. add options `-DWITH_CUDA=ON` and `-DWITH_GSTREAM=ON` in cmake  

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

# Usuage
Init SDK
------------------------------------
   `cd <install path>`  
   `sudo ./pcie_sdk_demo_init -s`   
   or use Python :  
   `sudo python init_sdk.py`  
   To use v4l2 video stream, init sdk as :  
   `sudo ./pcie_sdk_demo_init -v`   
   For video codec (with CUDA), init sdk as :  
   `sudo ./pcie_sdk_demo_init -d` (for display Only)  
   `sudo ./pcie_sdk_demo_init -r` (both display and save video file)   
   `sudo ./pcie_sdk_demo_init -w` (rtmp protocol)   

Set Sensor Config
------------------------------------
   `cd <src/python>`  
   for ALG sensor  
   `python set_sensor_config.py`   
   for non-ALG sensor (require config table)  
   `python set_sensor_config_from_file.py`  
   or use json file
   `python set_sensor_from_json.py --json_file=<path to json file> --channel=xx`  

Stream On/Off
------------------------------------
   `cd <src/python>`  
   Stream on
   `python stream_on.py`  
   or use
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

V4L2 Device
------------------------------------
   To capture video stream :  
   `gst-launch-1.0 v4l2src device=/dev/video0 ! rawvideoparse width=1920 height=1280 format=4 ! videoconvert ! autovideosink`  
   (You may want to change the parameters : device#, with, height,.. for your case)  

Stream Over RTMP
------------------------------------
   To capture video stream :  
   `gst-launch-1.0 rtmp2src location=rtmp://127.0.0.1:1935/live/ch00 ! decodebin ! queue ! videoconvert ! autovideosink sync=false`  

# Support
contact : jimmy@ailiteam.com
