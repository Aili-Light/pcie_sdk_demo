import algSDKpy
from algSDKpy import service_set_time
from algSDKpy import service_utc_time

cmd_topic = b"/service/set_time/get"
timeo = 5000

if __name__ == '__main__':
    cfg = service_set_time()
    cfg.ack_mode = 1
    cfg.dev_index = 1

    ret = algSDKpy.CallServices(cmd_topic, cfg, timeo)
    print(' result = %d, ack code [%d] '  % (ret, cfg.ack_code))
    print('---------Get Time-------------')
    print(' dev : %d, mode : %d, unix time : %ld, rel time : %ld' % (cfg.dev_index, cfg.time_mode, cfg.unix_time, cfg.relative_time))
    utc = cfg.utc_time
    year = (utc.year_month & 0x0FFF)
    month = ((utc.year_month >> 12) & 0x0F)
    day = (utc.day_wkday & 0x1F)
    hour = utc.hour
    minute = utc.minute
    second = utc.second
    print(' utc time : [yr:%d][mo:%d][dy:%d][h:%d][m:%d][s:%d]' % (year, month, day, hour, minute, second))

    print('---------finish-------------')
