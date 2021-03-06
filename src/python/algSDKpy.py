import os
import signal
import ctypes
from ctypes import *

ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE = 8192
ALG_SDK_SERVICE_SENSOR_PAYLOAD_SIZE = 7*ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE

ALG_SDK_MAX_CHANNEL = 16
ALG_SDK_MAX_DESERDES = 8

class service_camera_config(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("ch_id",c_uint8),
    ("module_type", c_uint16),
    ("width", c_uint16),
    ("height", c_uint16),
    ("deser_mode", c_uint8),
    ("camera_num", c_uint8),
    ("data_type", c_uint8),
    ("line_len", c_uint16),
    ("payload", c_uint8*ALG_SDK_SERVICE_SENSOR_PAYLOAD_SIZE),
    ("ack_code", c_uint8),
    ("channel", c_uint8)
    ]

class service_stream_control(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("select",c_uint8*ALG_SDK_MAX_CHANNEL),
    ("control", c_uint8*ALG_SDK_MAX_CHANNEL),
    ("ack_code", c_uint8),
    ("ch_sel", c_uint8*ALG_SDK_MAX_CHANNEL)
    ]

class pcie_common_head_t(Structure):
            _fields_ = [("head",c_uint8),
    ("version", c_uint8),
    ("topic_name",c_char*128),
    ("crc8", c_uint8),
    ("resv", c_uint8*125)
    ]

class pcie_image_info_meta_t(Structure):
            _fields_ = [("frame_index",c_uint32),
    ("width", c_uint16),
    ("height",c_uint16),
    ("data_type", c_uint16),
    ("exposure", c_float),
    ("again", c_float),
    ("dgain", c_float),
    ("temp", c_float),
    ("img_size", c_size_t),
    ("timestamp", c_uint64),
    ]

class pcie_image_data_t(Structure):
        _fields_ = [("common_head",pcie_common_head_t),
    ("image_info_meta",pcie_image_info_meta_t),
    ("payload", c_void_p),
    ]

callbackFunc_t = ctypes.CFUNCTYPE(c_void_p, c_void_p)

def CallServices(topic_ptr, cfg, timeo):
    pcie_sdk = ctypes.CDLL('../../pcie_sdk/lib/linux/libpcie_sdk.so')
    pcie_sdk.alg_sdk_call_service.argtypes = [c_char_p, c_void_p, c_int]
    pcie_sdk.alg_sdk_call_service.restype = ctypes.c_int

    ret = pcie_sdk.alg_sdk_call_service(c_char_p(topic_ptr), pointer(cfg), timeo)

    return ret

class algSDKInit():
    def __init__(self):
        self.pcie_sdk = ctypes.CDLL('../../pcie_sdk/lib/linux/libpcie_sdk.so')

    def InitSDK(self, frq):
        self.pcie_sdk.alg_sdk_init.argtypes = [c_int]
        self.pcie_sdk.alg_sdk_init.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_init(frq)

        return ret

    def Stop(self):
        print("stop")
        self.pcie_sdk.alg_sdk_stop()

    def Spin(self):
        self.pcie_sdk.alg_sdk_spin_on()


class algSDKClient():
    def __init__(self):
        self.pcie_sdk = ctypes.CDLL('../../pcie_sdk/lib/linux/libpcie_sdk.so')

    def InitClient(self):
        self.pcie_sdk.alg_sdk_init_client.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_init_client()

        return ret

    def Subscribe(self, topic_ptr, callback_func):
        self.pcie_sdk.alg_sdk_subscribe.argtypes = [c_char_p, callbackFunc_t]
        self.pcie_sdk.alg_sdk_subscribe.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_subscribe(topic_ptr, callback_func)

        return ret

    def Spin(self):
        self.pcie_sdk.alg_sdk_client_spin_on.restype = ctypes.c_int
        ret = self.pcie_sdk.alg_sdk_client_spin_on()

        return ret