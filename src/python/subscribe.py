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

def array2mat(w, h, payload, image_name):
    p_array = np.frombuffer(np.ctypeslib.as_array(payload, shape=((h*w*2, 1, 1))), dtype=np.uint8)
    img_in = p_array.reshape((h, w, 2))
    img_out = cv2.cvtColor(img_in, cv2.COLOR_YUV2BGR_YUYV)
    img_disp = cv2.resize(img_out, (640, 360))
    cv2.imshow(image_name, img_disp)
    cv2.waitKey(1)
    # print(image_name)

def CallbackFunc(ptr):
    p = ctypes.cast(ptr, ctypes.POINTER(pcie_image_data_t))
    payload = ctypes.cast(p.contents.payload, ctypes.POINTER(c_uint8))
    # print('[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]' % (p.contents.image_info_meta.frame_index, p.contents.image_info_meta.img_size, payload[0], payload[p.contents.image_info_meta.img_size-1]))
    array2mat(p.contents.image_info_meta.width, p.contents.image_info_meta.height, payload, p.contents.common_head.topic_name)

if __name__ == '__main__':
    callback_func = callbackFunc_t(CallbackFunc)
    client.Subscribe(topic, callback_func)
    ret = client.InitClient()
    if(ret < 0):
        print("Init Client Failed!")
        sys.exit(0)
    
    client.Spin()
