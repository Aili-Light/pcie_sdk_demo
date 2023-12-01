from ast import Not
import os
import sys
import argparse
import json
import time
import algSDKpy
from algSDKpy import service_camera_config
from algSDKpy import service_cam_pwr_reset

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Set sensor config by json file"
    )
    parser.add_argument('--json_file',
                        type=str,
                        help="path to json file",
                        required=True
                        )
    parser.add_argument('--channel_id',
                        type=int,
                        help="specify which channel the setting goes"
                        )
    parser.add_argument('-all_channels',
                        action="store_true",
                        help="Set this value true to apply same settings for all the channels"
                        )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
                        )
    parser.add_argument('--use_ups_config',
                        type=int,
                        help="Set this value true to apply no power config"
                        )
    parser.add_argument('--delay_time_ms',
                        type=int,
                        help="Control camera channel power reset time"
                        )
    parser.add_argument('--all_channels',
                    action="store_true",
                    help="Set this value true to apply same settings for all the channels"
                    )
    args = parser.parse_args()

    json_file = args.json_file
    is_set_all_ch = args.all_channels
    timeo = args.time_out

    try:
        with open(json_file, 'r') as fp:
            cfg_dict = json.load(fp)
            sensor_cfg = cfg_dict['sensor_cfg']
            sensor_name = sensor_cfg['sensor_name']
            sensor_width = int(sensor_cfg['sensor_width'])
            sensor_height = int(sensor_cfg['sensor_height'])
            config_table = sensor_cfg['config_table']
            # print ("sensor name is : %s, width=%d, height=%d, config table=%s" % (sensor_name, sensor_width, sensor_height, config_table))
            deserdes_cfg = cfg_dict['deserdes_cfg']
            channel_id = int(deserdes_cfg['channel_id'])
            des_mode = int(deserdes_cfg['des_mode'])
            camera_num = int(deserdes_cfg['camera_num'])
            data_type = int(deserdes_cfg['data_type'])
            # print ("deserdes : channel=%d, mode=%d, cam_num=%d,data_type=%d" % (channel_id, des_mode, camera_num, data_type))
    except IOError:
        print("Failed to open json file [%s]!" % json_file)
    else:
        cfg = service_camera_config()
        cfg.ack_mode = 1
        if args.channel_id is not None:
            cfg.ch_id = args.channel_id
            print("Set for channel %d" % cfg.ch_id)
        else:
            cfg.ch_id = channel_id

        if sensor_name == "alg_ov_ox08b":
            cfg.module_type = int(b"0x000C", 16)
        elif sensor_name == "alg_ov_ov03c":
            cfg.module_type = int(b"0x000B", 16)
        elif sensor_name == "alg_sony_isx031":
            cfg.module_type = int(b"0x0002", 16)
        elif sensor_name == "alg_sony_isx019":
            cfg.module_type = int(b"0x0001", 16)
        else:
            cfg.module_type = int(b"0xFFFF", 16)

        if  args.use_ups_config == 1:
            print(' use ups mode to download config ')
            cfg.module_type = int(b"0x9999", 16)
        if  args.delay_time_ms != None:
            print(' use delay param to ctrl camera power ')
            cmd_topic = b"/service/cam/ch_pwr_ctrl"

            cam_pwr_cfg = service_cam_pwr_reset()
            cam_pwr_cfg.ack_mode = 1
            cam_pwr_cfg.board_id = int(args.channel_id/4)
            cam_pwr_cfg.ch_id = (args.channel_id%4)*2
            cam_pwr_cfg.pwr_ctrl_sts = 0

            ret = algSDKpy.CallServices(cmd_topic, cam_pwr_cfg, timeo)
            print(' camera power1 off result = %d '% ret)

            cam_pwr_cfg.ack_mode = 1
            cam_pwr_cfg.board_id = int(args.channel_id/4)
            cam_pwr_cfg.ch_id = (args.channel_id%4)*2+1
            cam_pwr_cfg.pwr_ctrl_sts = 0

            ret = algSDKpy.CallServices(cmd_topic, cam_pwr_cfg, timeo)
            print(' camera power2 off result = %d '% ret)

            delay_time_s = float(args.delay_time_ms)/1000
            print(' delay_time: %f s'% delay_time_s)
            time.sleep(delay_time_s)
            cam_pwr_cfg.ack_mode = 1
            cam_pwr_cfg.board_id = int(args.channel_id/4)
            cam_pwr_cfg.ch_id = (args.channel_id%4)*2
            cam_pwr_cfg.pwr_ctrl_sts = 1

            ret = algSDKpy.CallServices(cmd_topic, cam_pwr_cfg, timeo)
            print(' camera power1 on result = %d '% ret)

            cam_pwr_cfg.ack_mode = 1
            cam_pwr_cfg.board_id = int(args.channel_id/4)
            cam_pwr_cfg.ch_id = (args.channel_id%4)*2+1
            cam_pwr_cfg.pwr_ctrl_sts = 1

            ret = algSDKpy.CallServices(cmd_topic, cam_pwr_cfg, timeo)
            print(' camera power2 on result = %d '% ret)

        cfg.width = sensor_width
        cfg.height = sensor_height
        cfg.deser_mode = des_mode
        cfg.camera_num = camera_num
        cfg.data_type = data_type

        try:
            with open(config_table, "r") as filestream:
                line_num = 0
                for line in filestream:
                    line_split = line.split(",")
                    # print("%s, %d" % (line_split[0], int(line_split[0],16)))
                    cfg.payload[7*line_num] = int(line_split[0], 16)
                    cfg.payload[7*line_num+1] = (int(line_split[1], 16) & 0xFF)
                    cfg.payload[7*line_num+2] = (int(line_split[1], 16) >> 8)
                    cfg.payload[7*line_num+3] = (int(line_split[2], 16) & 0xFF)
                    cfg.payload[7*line_num+4] = (int(line_split[2], 16) >> 8)
                    cfg.payload[7*line_num+5] = (int(line_split[3], 16) & 0xFF)
                    cfg.payload[7*line_num+6] = (int(line_split[3], 16) >> 8)
                    line_num = line_num + 1

            cfg.line_len = line_num
            # For Debug
            # print("line num = %d" % line_num)
            # for i in range(0, line_num):
            #     print("%d, %d, %d, %d, %d, %d, %d" % (cfg.payload[7*i], cfg.payload[7*i+1], cfg.payload[7*i+2], cfg.payload[7*i+3], cfg.payload[7*i+4], cfg.payload[7*i+5], cfg.payload[7*i+6]))
        except IOError:
            print("Failed to load config table [%s]!" % config_table)
        else:
            cmd_topic = b"/service/camera/set_config"
            if is_set_all_ch is True:
                for i in range(0, 4):
                    cfg.ch_id = i
                    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
                    print(' result = %d ' % ret)
            else:
                ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
                print(' result = %d ' % ret)

    print('---------finish-------------')
