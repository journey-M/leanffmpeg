//
// Created by guoweijie004 on 21-3-4.
//

#ifndef LEANFFMPEG_CLOCK_H
#define LEANFFMPEG_CLOCK_H

extern "C" {
#include <libavutil/time.h>

}


class Clock {

public:

    double pts;           /* clock base */
    double pts_drift;     /* clock base minus time at which we updated the clock */
    double last_updated;
    double speed;
//    int serial;           /* clock is based on a packet with this serial */
    int paused;
//    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */


    double get_clock();

    void set_clock_at(double pts, double time);

    void set_clock(double pts);

    void set_clock_speed(double speed);

    void init_clock();

//    void sync_clock_to_slave(Clock *c, Clock *slave);

};


#endif //LEANFFMPEG_CLOCK_H
