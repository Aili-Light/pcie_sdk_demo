import algSDKpy
from algSDKpy import service_set_time
from algSDKpy import service_utc_time

cmd_topic = b"/service/set_time/set"
timeo = 5000

if __name__ == '__main__':
    year = 2022
    month = 8
    day = 23
    wk_day = 5
    utc = service_utc_time()
    utc.year_month = (month<<12) | (year & 0x0FFF)
    utc.day_wkday = (wk_day<<5) | (day & 0x1F)
    utc.hour = 15
    utc.minute = 30
    utc.second = 23
    utc.us = 12565253

    cfg = service_set_time()
    cfg.ack_mode = 1
    cfg.dev_index = 1
    cfg.time_mode = 1  # 1=utc 2=unix 4=relative
    cfg.unix_time = 16586335333000000
    cfg.relative_time = 0
    cfg.utc_time = utc

    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d ' % cfg.ack_code)

    print('---------finish-------------')
