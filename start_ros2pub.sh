#!/bin/bash
export LD_LIBRARY_PATH=/opt/ros/humble/lib/:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

./release/linux/bin/pcie_sdk_demo_ros2pub -c /image_data/stream/ch_00
