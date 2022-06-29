import signal, sys
import time
import ctypes
from ctypes import *

import algSDKpy
from algSDKpy import algSDKClient
from algSDKpy import pcie_image_data_t
from algSDKpy import callbackFunc_t

client = algSDKClient()
topic = b"/image_data/stream/00"

def CallbackFunc(ptr):
    p = ctypes.cast(ptr, ctypes.POINTER(pcie_image_data_t))
    payload = ctypes.cast(p.contents.payload, ctypes.POINTER(c_uint8))
    print('[frame = %d], [time %ld], [byte_0 = %d], [byte_end = %d]' % (p.contents.image_info_meta.frame_index, p.contents.image_info_meta.img_size, payload[0], payload[p.contents.image_info_meta.img_size-1]))

if __name__ == '__main__':
    callback_func = callbackFunc_t(CallbackFunc)
    client.Subscribe(topic, callback_func)
    ret = client.InitClient()
    if(ret < 0):
        print("Init Client Failed!")
        sys.exit(0)
    
    client.Spin()
