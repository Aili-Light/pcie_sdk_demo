import argparse
import algSDKpy
from algSDKpy import service_get_cam_emb_info
from algSDKpy import cam_emb_param

cmd_topic = b"/service/camera/get_emb_info"
timeo = 5000

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Get camera ch emb info"
    )
    parser.add_argument('--device',
                        type=int,
                        help="device id to get camera ch emb info (e.g. --device=0)",
                        required=True
    )
    parser.add_argument('--channel',
                        type=int,
                        help="channel id to get camera ch emb info (e.g. --channel=0 value[0...3] or 0xff)",
                        required=True
    )
    parser.add_argument('--time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )
    args = parser.parse_args()

    cfg = service_get_cam_emb_info()
    cfg.ack_mode = 1
    cfg.dev_index = args.device
    cfg.channel = args.channel
    channel = args.channel
    dev = args.device
    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d, ack code [%d] '  % (ret, cfg.ack_code))
    if cfg.ack_code==0 :
        print('---------Get ch emb param-------------')
        if channel==0xff :
            for i in range(4) :
                print(' dev : %d, channel : %d, valid : %d, hsize : %d, f_vsize : %d, rear_vsize %d' % (dev, i,cfg.emb_param[i].valid, cfg.emb_param[i].non_image_h_size,cfg.emb_param[i].non_image_front_v_size,cfg.emb_param[i].non_image_rear_v_size))
        else :
            print(' dev : %d, channel : %d, valid : %d, hsize : %d, f_vsize : %d, rear_vsize %d' % (dev, channel,cfg.emb_param[channel].valid, cfg.emb_param[channel].non_image_h_size,cfg.emb_param[channel].non_image_front_v_size,cfg.emb_param[channel].non_image_rear_v_size))
    else :
        print('Failed to emb param [device:%d][Ch:%d]. Errcode : %d' % (cfg.dev_index, channel, cfg.ack_code))

    print('---------finish-------------')
