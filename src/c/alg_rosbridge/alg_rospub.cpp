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
#include "alg_rospub.h"
#include "alg_common/basic_types.h"

const char topic_image_head_t[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = {ALG_SDK_TOPIC_NAME_IMAGE_DATA};

AlgRosPubNode::AlgRosPubNode() {}

AlgRosPubNode::~AlgRosPubNode() {}

int AlgRosPubNode::Init(const int _ch_id)
{
    m_ch_id = _ch_id;

    ros::NodeHandle nh;
    ros::NodeHandle pnh("~");

    t_init_ = ros::Time::now();

    char topic_ch[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = "";
    char topic_name[ALG_SDK_HEAD_COMMON_TOPIC_NAME_LEN] = "";
    sprintf(topic_ch, "/%02d", this->m_ch_id);
    strcat(topic_name, topic_image_head_t);
    strcat(topic_name, topic_ch);
    std::string topic_name_s = topic_name;

    printf("Initialized ROS Topic : [%s]\n", topic_name_s.c_str());
    m_img_pub = nh.advertise<sensor_msgs::Image>(topic_name_s, 1000);

    return 0;
}

bool AlgRosPubNode::IsRosOK()
{
    return ros::ok();
}

void AlgRosPubNode::SpinOnce()
{
    ros::spinOnce();
}

int AlgRosPubNode::PublishImage(uint32_t seq, 
                                std::string topic_name, 
                                int height, 
                                int width, 
                                int format, 
                                size_t img_size, 
                                uint64_t timestamp,
                                void* _buffer
                                )
{
    if (!_buffer || img_size==0)
    {
        printf("ROS Pub Error : Buffer is empty!\n");
        return 1;
    }
    uint8_t* buffer = (uint8_t*)_buffer;
    sensor_msgs::Image ros_image;
    ros::Time ros_time;
    ros_time.fromNSec(timestamp*1000); // timestamp is macrosecond

    ros_image.header.seq = seq;
    ros_image.header.stamp = ros_time;
    ros_image.header.frame_id = topic_name;

    if (format == ALG_SDK_VIDEO_FORMAT_YVYU ||
        format == ALG_SDK_VIDEO_FORMAT_YUY2 || 
        format == ALG_SDK_VIDEO_FORMAT_UYVY || 
        format == ALG_SDK_VIDEO_FORMAT_VYUY
    )
    {
        /* The YUV Family */
        ros_image.height = height;
        ros_image.width = width;
        ros_image.encoding = "yuv422";
        ros_image.step = 2*width;
        ros_image.is_bigendian = false;
        /* ROS image data is a std Vector 
           therefore we must use memcpy to fill it
        */
        ros_image.data.resize(img_size);
        memcpy(ros_image.data.data(), buffer, img_size);
        // printf("----b0[%d], bend[%d]\n", ((uint8_t*)buffer)[0],
        //  ((uint8_t*)buffer)[metadata->img_size-1]);
        // printf("vector size : %ld\n", ros_image.data.size());
        /* Publish the Image */
        m_img_pub.publish(ros_image);
        /* Update ROS */
        SpinOnce();
    }
    else if (format == ALG_SDK_VIDEO_FORMAT_RGB || 
             format == ALG_SDK_VIDEO_FORMAT_BGR
    )
    {
        ros_image.height = height;
        ros_image.width = width;
        ros_image.step = 3*width;
        ros_image.is_bigendian = false;  

        switch (format)
        {
        case ALG_SDK_VIDEO_FORMAT_RGB :
            ros_image.encoding = "rgb8";
            break;
        case ALG_SDK_VIDEO_FORMAT_BGR :
            ros_image.encoding = "bgr8";
            break;
        default:
            break;
        }

        ros_image.data.resize(img_size);
        memcpy(ros_image.data.data(), buffer, img_size);
        // printf("----b0[%d], bend[%d]\n", ((uint8_t*)buffer)[0],
        //  ((uint8_t*)buffer)[img_size-1]);
        // printf("vector size : %ld\n", ros_image.data.size());
        m_img_pub.publish(ros_image);
        /* Update ROS */
        SpinOnce();
    }
    else if(format == ALG_SDK_VIDEO_FORMAT_GRAY8)
    {
        ros_image.height = height;
        ros_image.width = width;
        ros_image.step = width;
        ros_image.is_bigendian = false;  
        ros_image.encoding = "mono8";
        ros_image.data.resize(img_size);
        memcpy(ros_image.data.data(), buffer, img_size);
        // printf("----b0[%d], bend[%d]\n", ((uint8_t*)buffer)[0],
        //  ((uint8_t*)buffer)[img_size-1]);
        // printf("vector size : %ld\n", ros_image.data.size());
        m_img_pub.publish(ros_image);
        /* Update ROS */
        SpinOnce();
    }
    else
    {
        printf( "ROS Publisher -- unknown data type (%d)\n", format);
        printf( "                        supported data type are:\n");
        printf( "                            * 4 (YVYU)\n");		
        printf( "                            * 5 (UYVY)\n");		
        printf( "                            * 64 (VYUY)\n");		
        printf( "                            * 19 (YVYU)\n");		
        printf( "                            * 15 (RGB)\n");		
        printf( "                            * 16 (BRG)\n");		
        return 1; 
    }

    return 0;
}
