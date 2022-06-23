import os
import ctypes
from ctypes import *

ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE = 8192
ALG_SDK_SERVICE_SENSOR_PAYLOAD_SIZE = 7*ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE

ALG_SDK_MAX_CHANNEL = 16

class service_camera_config(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("ch_id",c_uint8),
    ("module_type", c_uint16),
    ("width", c_uint16),
    ("height", c_uint16),
    ("deser_mode", c_uint8),
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

def CallServices(topic_ptr, cfg_ptr, timeo):
    pcie_sdk = ctypes.CDLL('../../pcie_sdk/lib/linux/libpcie_sdk.so')
    pcie_sdk.alg_sdk_call_service.argtypes = [c_char_p, c_void_p, c_int]
    pcie_sdk.alg_sdk_call_service.restype = ctypes.c_int

    ret = pcie_sdk.alg_sdk_call_service(c_char_p(topic_ptr), pointer(cfg_ptr), timeo)

    return ret
