/*
 The MIT License (MIT)

Copyright (c) 2022 Aili-Light. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef __ALG_ROS2PUB_H__
#define __ALG_ROS2PUB_H__

#include <stdlib.h>
#include <string.h>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"

class AlgRos2PubNode
{
public:
    AlgRos2PubNode                      ();
    ~AlgRos2PubNode                     ();
    int  Init                           (const int ch_id);
    bool IsRosOK                        ();
    void SpinOnce                       ();
    int  PublishImage                   (uint32_t seq, 
                                        std::string topic_name, 
                                        int height, 
                                        int width, 
                                        int format, 
                                        size_t img_size, 
                                        uint64_t timestamp,
                                        void* _buffer
                                        );

private:
    int                                 m_freq;
    int                                 m_ch_id;
    int                                 m_image_width;
    int                                 m_image_height;
    int                                 m_image_format;
    int                                 t_current_sec_;
    int                                 t_current_nsec_;
    size_t                              count_;

private:
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_img_pub;
    rclcpp::Node::SharedPtr             m_node;
};

#endif