import argparse
import algSDKpy
from algSDKpy import service_cam_pwr_reset

cmd_topic = b"/service/cam/ch_pwr_ctrl"
timeo = 5000

if __name__ == '__main__':
    # for i in range(0, 4):
    parser = argparse.ArgumentParser(
        description="Set camera power reset"
    )
    parser.add_argument('--board_id',
                        type=int,
                        help="specify which board the setting goes",
                        required=True
                        )
    parser.add_argument('--channel_id',
                        type=str,
                        help="specify which channel the setting goes",
                        required=True
                        )
    parser.add_argument('--ch_pwr_sts',
                        type=int,
                        help="control camera channel power status",
                        required=True
                        )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
                        )
    args = parser.parse_args()

    cfg = service_cam_pwr_reset()
    channel = args.channel_id
    ch_split = channel.split(",")
    timeo = args.time_out

    for item in ch_split:
        ch_id = int(item)
        if(ch_id < 8):
            cfg.ack_mode = 1
            cfg.board_id = args.board_id
            cfg.ch_id = ch_id
            cfg.pwr_ctrl_sts = args.ch_pwr_sts
            ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
            print(' result = %d ' % ret)

    print('---------finish-------------')
