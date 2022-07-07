import signal, sys
import time
import numpy as np
import ctypes
from ctypes import *
import cv2

import algSDKpy
from algSDKpy import algSDKClient
from algSDKpy import pcie_image_data_t
from algSDKpy import callbackFunc_t

client = algSDKClient()
topic = b"/image_data/stream"

def array2mat(w, h, payload, image_name, ch_id, frame_index, type):
    p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((h*w*2, 1, 1))), dtype=np.uint8)
    
    if (type == 0):
        img_in = p_array.reshape((h, w, 2))
        img_out = cv2.cvtColor(img_in, cv2.COLOR_YUV2BGR_YUYV)
        img_disp = cv2.resize(img_out, (640, 360))
        cv2.imshow(str(image_name), img_disp)
        
        filename = str("image_")+str("%02d_" % ch_id)+str("%08d_" % frame_index)+str(".bmp")
        c = cv2.waitKey(1)
        if c == 32:
            cv2.imwrite(filename, img_out)

    elif (type == 1):
        p_data = np.zeros(shape=(h*w, 1, 1), dtype=np.uint16)
        for i in range(0, int(h*w/4)):
            p_data[4*i] = (((np.ushort(p_array[5*i]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 0) & 0x0003));
            p_data[4*i+1] = (((np.ushort(p_array[5*i+1]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 2) & 0x0003));
            p_data[4*i+2] = (((np.ushort(p_array[5*i+2]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 4) & 0x0003));
            p_data[4*i+3] = (((np.ushort(p_array[5*i+3]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 6) & 0x0003));
        
        # filename = str("image_")+str("%02d_" % ch_id)+str("%08d_" % frame_index)+str(".raw")
        # np.save(filename, p_data)

def get_channel_id(topic_name):
    topic = str(topic_name)
    tx = topic.split("/")
    ch_id = int(tx[3])
    
    return ch_id

def CallbackFunc(ptr):
    p = ctypes.cast(ptr, ctypes.POINTER(pcie_image_data_t))
    payload = ctypes.cast(p.contents.payload, ctypes.POINTER(c_uint8))
    # print('[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]' % (p.contents.image_info_meta.frame_index, p.contents.image_info_meta.img_size, payload[0], payload[p.contents.image_info_meta.img_size-1]))
    array2mat(p.contents.image_info_meta.width, p.contents.image_info_meta.height, payload, p.contents.common_head.topic_name, get_channel_id(p.contents.common_head.topic_name), p.contents.image_info_meta.frame_index, 0)

if __name__ == '__main__':
    callback_func = callbackFunc_t(CallbackFunc)
    client.Subscribe(topic, callback_func)
    ret = client.InitClient()
    if(ret < 0):
        print("Init Client Failed!")
        sys.exit(0)
    
    client.Spin()
