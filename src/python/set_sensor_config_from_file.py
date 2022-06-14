import os
import algSDKpy
from algSDKpy import service_camera_config

cmd_topic = b"/service/camera/set_config"
timeo = 5000
# config_file = b"../../config/alg_config_ov_0x8b.txt"
config_file = b"../../config/alg_config_sony_isx019.txt"
channel = 0
sensor_width = 1280
sensor_height = 960

if __name__ == '__main__':

    cfg = service_camera_config()
    cfg.ack_mode = 1
    cfg.ch_id = channel
    cfg.module_type = int(b"0xFFFF",16)
    cfg.width = sensor_width
    cfg.height = sensor_height
    cfg.deser_mode = 1

    with open(config_file,"r") as filestream:
        line_num = 0
        for line in filestream:
            line_split = line.split(",")
            # print("%s, %d" % (line_split[0], int(line_split[0],16)))
            cfg.payload[7*line_num] = int(line_split[0],16)
            cfg.payload[7*line_num+1] = (int(line_split[1],16) & 0xFF)
            cfg.payload[7*line_num+2] = (int(line_split[1],16) >> 8)
            cfg.payload[7*line_num+3] = (int(line_split[2],16) & 0xFF)
            cfg.payload[7*line_num+4] = (int(line_split[2],16) >> 8)
            cfg.payload[7*line_num+5] = (int(line_split[3],16) & 0xFF)
            cfg.payload[7*line_num+6] = (int(line_split[3],16) >> 8)
            line_num = line_num + 1

    cfg.line_len = line_num

    # For Debug
    # print("line num = %d" % line_num)
    # for i in range(0, line_num):
    #     print("%d, %d, %d, %d, %d, %d, %d" % (cfg.payload[7*i], cfg.payload[7*i+1], cfg.payload[7*i+2], cfg.payload[7*i+3], cfg.payload[7*i+4], cfg.payload[7*i+5], cfg.payload[7*i+6]))

    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d ' % ret)

    print('---------finish-------------')
