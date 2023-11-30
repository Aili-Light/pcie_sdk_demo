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
#ifndef __ALG_ROSPUB_H__
#define __ALG_ROSPUB_H__

#include <stdlib.h>
#include <string.h>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>

class AlgRosPubNode
{
public:
    AlgRosPubNode                       ();
    ~AlgRosPubNode                      ();
    int  Init                           (int, int);
    bool IsRosOK                        ();
    void Spin                           ();
    void SpinOnce                       ();
    int  GetImageWidth                  () const;
    int  GetImageHeight                 () const;
    int  GetImageFormat                 () const;

public:
    void SetImageWidth                  (const int);
    void SetImageHeight                 (const int);
    void SetImageFormat                 (const int);
    int  PublishImage                   (uint32_t , std::string , int , int , size_t , int, void* );

private:
    int                                 m_freq;
    int                                 m_ch_id;
    int                                 m_image_width;
    int                                 m_image_height;
    int                                 m_image_format;
    int                                 t_current_sec_;
    int                                 t_current_nsec_;

private:
    ros::Publisher                      m_img_pub;
    ros::Time                           t_init_;
};

#endif