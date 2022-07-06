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

def array2mat(w, h, payload, image_name, type):
    p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((h*w*2, 1, 1))), dtype=np.uint8)
    is_save = 0
    
    if (type == 0):
        img_in = p_array.reshape((h, w, 2))
        img_out = cv2.cvtColor(img_in, cv2.COLOR_YUV2BGR_YUYV)
        img_disp = cv2.resize(img_out, (640, 360))
        cv2.imshow(str(image_name), img_disp)
        if (is_save == 1):
            cv2.imsave("test.jpg", img_disp)
        
        cv2.waitKey(1)
    elif (type == 1):
        p_data = np.zeros(shape=(h*w*2, 1, 1), dtype=np.uint8)
        for i in range(0, int(h*w/4)):
            # print(np.ushort(p_array[5*i]))
            p_data[4*i] = (((np.ushort(p_array[5*i]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 0) & 0x0003));
            p_data[4*i+1] = (((np.ushort(p_array[5*i+1]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 2) & 0x0003));
            p_data[4*i+2] = (((np.ushort(p_array[5*i+2]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 4) & 0x0003));
            p_data[4*i+3] = (((np.ushort(p_array[5*i+3]) << 2) & 0x03FC) | np.ushort((p_array[5*i+4] >> 6) & 0x0003));
        
        if (is_save == 1):
            np.save("test.raw", p_data)
            print("saved image")

def CallbackFunc(ptr):
    p = ctypes.cast(ptr, ctypes.POINTER(pcie_image_data_t))
    payload = ctypes.cast(p.contents.payload, ctypes.POINTER(c_uint8))
    # print('[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]' % (p.contents.image_info_meta.frame_index, p.contents.image_info_meta.img_size, payload[0], payload[p.contents.image_info_meta.img_size-1]))
    array2mat(p.contents.image_info_meta.width, p.contents.image_info_meta.height, payload, p.contents.common_head.topic_name, 0)

if __name__ == '__main__':
    callback_func = callbackFunc_t(CallbackFunc)
    client.Subscribe(topic, callback_func)
    ret = client.InitClient()
    if(ret < 0):
        print("Init Client Failed!")
        sys.exit(0)
    
    client.Spin()
