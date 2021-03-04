#include "Clock.h"

#  define NAN (0.0f / 0.0f)
#  define isnan(x) __builtin_isnan (x)

double Clock::get_clock() {
//    if (*this->queue_serial != serial)
//        return NAN;
    if (paused) {
        return pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        return pts_drift + time - (time - last_updated) * (1.0 - speed);
    }
}

void Clock::set_clock_at(double pts, double time) {
    this->pts = pts;
    this->last_updated = time;
    this->pts_drift = this->pts - time;
}

void Clock::set_clock(double pts) {
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(pts, time);
}

void Clock::set_clock_speed(double speed) {
    this->speed = speed;
}

void Clock::init_clock() {
    this->speed = 1.0;
    this->paused = 0;
    set_clock(NAN);
}

//void Clock::sync_clock_to_slave(Clock *c, Clock *slave) {
//    double clock = get_clock(c);
//    double slave_clock = get_clock(slave);
//    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
//        set_clock(c, slave_clock, slave->serial);
//}
