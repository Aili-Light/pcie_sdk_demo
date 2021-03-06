import algSDKpy
from algSDKpy import service_stream_control

cmd_topic = b"/service/camera/stream_on"
timeo = 5000

if __name__ == '__main__':
    cam_ctl = service_stream_control()
    cam_ctl.ack_mode = 1

    for i in range(0, 16):
        cam_ctl.select[i] = 0
        cam_ctl.control[i] = 0

    cam_ctl.select[0] = 1
    cam_ctl.control[0] = 1
    cam_ctl.select[1] = 1
    cam_ctl.control[1] = 0
    cam_ctl.select[2] = 1
    cam_ctl.control[2] = 1
    cam_ctl.select[3] = 1
    cam_ctl.control[3] = 0
    cam_ctl.select[4] = 1
    cam_ctl.control[4] = 1
    cam_ctl.select[5] = 1
    cam_ctl.control[5] = 0
    cam_ctl.select[6] = 1
    cam_ctl.control[6] = 1
    cam_ctl.select[7] = 1
    cam_ctl.control[7] = 0

    ret = algSDKpy.CallServices(cmd_topic, cam_ctl, timeo)
    print(' result = %d ' % ret)

    print('---------finish-------------')
