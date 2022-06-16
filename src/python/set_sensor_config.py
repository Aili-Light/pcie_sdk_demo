import algSDKpy

cmd_topic = b"/service/camera/set_config"
timeo = 5000

if __name__ == '__main__':
    for i in range(0, 4):
        cfg = algSDKpy.service_camera_config()
        cfg.ack_mode = 1
        cfg.ch_id = i
        cfg.module_type = 12

        ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
        print(' result = %d ' % ret)

    print('---------finish-------------')
