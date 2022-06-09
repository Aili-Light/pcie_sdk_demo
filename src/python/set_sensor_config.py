import os
import ctypes
from ctypes import *

ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE = 1024
ALG_SDK_MAX_CHANNEL = 32

class service_camera_config(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("ch_id",c_uint8),
    ("module_type", c_uint16),
    ("width", c_uint16),
    ("height", c_uint16),
    ("line_len", c_uint16),
    ("payload", c_uint8*7*ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE),
    ("ack_code", c_uint8),
    ("channel", c_uint8)
    ]

class service_stream_controol(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("select",c_uint8*ALG_SDK_MAX_CHANNEL),
    ("control", c_uint8*ALG_SDK_MAX_CHANNEL),
    ("ack_code", c_uint8),
    ("ch_sel", c_uint8*ALG_SDK_MAX_CHANNEL)
    ]

set_config_cmd = b"/service/camera/set_config"
timeo = c_int(5000)

if __name__ == '__main__':
    pcie_sdk = ctypes.CDLL('../../lib/linux/libpcie_sdk.so')
    pcie_sdk.alg_sdk_call_service.argtypes = [c_char_p, c_void_p, c_int]
    pcie_sdk.alg_sdk_call_service.restype = ctypes.c_int

    for i in range(0, (ALG_SDK_MAX_CHANNEL-1)):
        cfg = service_camera_config()
        cfg.ack_mode = 1
        cfg.ch_id = i
        cfg.module_type = 12
        ret = pcie_sdk.alg_sdk_call_service(c_char_p(set_config_cmd), pointer(cfg), timeo)
        print(' result = %d ' % ret)

    print('---------finish-------------')
