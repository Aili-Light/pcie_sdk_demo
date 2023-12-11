#!/bin/bash
export LD_LIBRARY_PATH=/opt/ros/noetic/lib/:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

#./release/linux/bin/pcie_sdk_demo_rospub -c /image_data/stream/00
./release/linux/bin/pcie_sdk_demo_rospub -all