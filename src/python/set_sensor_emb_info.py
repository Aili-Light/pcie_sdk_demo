import argparse
import algSDKpy
from algSDKpy import service_set_cam_emb_info
from algSDKpy import cam_emb_param

cmd_topic = b"/service/camera/set_emb_info"
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
    parser.add_argument('--valid',
                        type=int ,
                        help="cam emb whether enable 0:disable 1:enable (e.g. --valid=1)",
                        required=True
    )
    parser.add_argument('--hsize',
                        type=int ,
                        help="set emb data h size(e.g. --hsize=2748)",
                        required=True
    )
    parser.add_argument('--fvsize',
                        type=int ,
                        help="set emb data front v size (e.g. --f_vsize=2)",
                        required=True
    )
    parser.add_argument('--rvsize',
                        type=int ,
                        help="set emb data rear v size (e.g. --r_vsize=2)",
                        required=True
    )
    parser.add_argument('--time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )
    
    args = parser.parse_args()

    cfg = service_set_cam_emb_info()
    cfg.ack_mode = 1
    cfg.dev_index = args.device
    cfg.channel = args.channel
    channel = args.channel
    dev = args.device
    if channel==0xff :
        for i in range(4) :
            cfg.emb_param[i].valid = args.valid
            cfg.emb_param[i].non_image_h_size = args.hsize
            cfg.emb_param[i].non_image_front_v_size = args.fvsize
            cfg.emb_param[i].non_image_rear_v_size = args.rvsize

    else :
        cfg.emb_param[channel].valid = args.valid
        cfg.emb_param[channel].non_image_h_size = args.hsize
        cfg.emb_param[channel].non_image_front_v_size = args.fvsize
        cfg.emb_param[channel].non_image_rear_v_size = args.rvsize
    
    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d, ack code [%d] '  % (ret, cfg.ack_code))
    if cfg.ack_code==0 and ret == 0:
        print('Sucessful to set emb param [device:%d][Ch:%d]. Errcode : %d' % (cfg.dev_index, channel, cfg.ack_code))
    else :
        print('Failed to set emb param [device:%d][Ch:%d]. Errcode : %d' % (cfg.dev_index, channel, cfg.ack_code))

    print('---------finish-------------')
