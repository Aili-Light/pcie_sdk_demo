import os
import ctypes
from ctypes import *

ALG_SDK_SERVICE_SENSOR_CONFIG_MAX_LINE = 1024
ALG_SDK_MAX_CHANNEL = 32

class service_stream_control(Structure):
    _fields_ = [("ack_mode",c_uint8),
    ("select",c_uint8*ALG_SDK_MAX_CHANNEL),
    ("control", c_uint8*ALG_SDK_MAX_CHANNEL),
    ("ack_code", c_uint8),
    ("ch_sel", c_uint8*ALG_SDK_MAX_CHANNEL)
    ]

stream_on_cmd = b"/service/camera/stream_on"
timeo = c_int(5000)

if __name__ == '__main__':
    pcie_sdk = ctypes.CDLL('../../lib/libpcie_sdk.so')
    pcie_sdk.alg_sdk_call_service.argtypes = [c_char_p, c_void_p, c_int]
    pcie_sdk.alg_sdk_call_service.restype = ctypes.c_int

    cfg = service_stream_control()
    cfg.ack_mode = 1

    for i in range(0, (ALG_SDK_MAX_CHANNEL-1)):
        cfg.select[i] = 0
        cfg.control[i] = 0

    cfg.select[0] = 1
    cfg.control[0] = 1
    cfg.select[2] = 1
    cfg.control[2] = 1
    cfg.select[4] = 1
    cfg.control[4] = 1
    cfg.select[6] = 1
    cfg.control[6] = 1

    ret = pcie_sdk.alg_sdk_call_service(c_char_p(stream_on_cmd), pointer(cfg), timeo)
    print(' result = %d ' % ret)

    print('---------finish-------------')
