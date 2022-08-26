import argparse, sys
import algSDKpy
from algSDKpy import service_trigger_slvcmd
from algSDKpy import service_set_trigger

cmd_topic = b"/service/set_trigger/set"
timeo = 5000

if __name__ == '__main__':
    cfg = service_set_trigger()
    cfg.ack_mode = 1
    cfg.set_mode = 0    # 0=set trigger param, 1=set trigger mode 
    cfg.trigger_mode = 2 # 0=No_TRG, 1=Ext_TRG, 2=Int_TRG
    cfg.master_trigger_freq = 25 # master trigger mode frequency

    for i in range(0, 16):
        cfg.select[i] = 0

    cfg.select[0] = 1  # for device 0
    cfg.select[8] = 0  # for device 1

    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d, ack code [%d]' % (ret, cfg.ack_code))

    print('---------finish-------------')
