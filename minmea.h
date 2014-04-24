/*
 * Copyright © 2014 Kosma Moczek <kosma@cloudyourcar.com>
 * This program is free software. It comes without any warranty, to the extent
 * permitted by applicable law. You can redistribute it and/or modify it under
 * the terms of the Do What The Fuck You Want To Public License, Version 2, as
 * published by Sam Hocevar. See the COPYING file for more details.
 */

#ifndef MINMEA_H
#define MINMEA_H

#define _BSD_SOURCE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#define MINMEA_MAX_LENGTH 80

enum minmea_sentence_id {
    MINMEA_INVALID = -1,
    MINMEA_UNKNOWN = 0,
    MINMEA_SENTENCE_RMC,
    MINMEA_SENTENCE_GGA,
    MINMEA_SENTENCE_GSA,
    MINMEA_SENTENCE_GST,
    MINMEA_SENTENCE_GSV,
};

struct minmea_date {
    int day;
    int month;
    int year;
};

struct minmea_time {
    int hours;
    int minutes;
    int seconds;
    int microseconds;
};

struct minmea_sentence_rmc {
    struct minmea_time time;
    bool valid;
    int latitude, latitude_scale;
    int longitude, longitude_scale;
    int speed, speed_scale;
    int course, course_scale;
    struct minmea_date date;
    int variation, variation_scale;
};

struct minmea_sentence_gga {
    struct minmea_time time;
    int latitude, latitude_scale;
    int longitude, longitude_scale;
    int fix_quality;
    int satellites_tracked;
    int hdop, hdop_scale;
    int altitude, altitude_scale; char altitude_units;
    int height, height_scale; char height_units;
    int dgps_age;
};

struct minmea_sentence_gst {
    struct minmea_time time;
    int rms_deviation, rms_deviation_scale;
    int semi_major_deviation, semi_major_deviation_scale;
    int semi_minor_deviation, semi_minor_deviation_scale;
    int semi_major_orientation, semi_major_orientation_scale;
    int latitude_error_deviation, latitude_error_deviation_scale;
    int longitude_error_deviation, longitude_error_deviation_scale;
    int altitude_error_deviation, altitude_error_deviation_scale;
};

enum minmea_gsa_mode {
    MINMEA_GPGSA_MODE_AUTO = 'A',
    MINMEA_GPGSA_MODE_FORCED = 'M',
};

enum minmea_gsa_fix_type {
    MINMEA_GPGSA_FIX_NONE = 1,
    MINMEA_GPGSA_FIX_2D = 2,
    MINMEA_GPGSA_FIX_3D = 3,
};

struct minmea_sentence_gsa {
    char mode;
    int fix_type;
    int sats[12];
    int pdop, pdop_scale;
    int hdop, hdop_scale;
    int vdop, vdop_scale;
};

struct minmea_sat_info {
    int nr;
    int elevation;
    int azimuth;
    int snr;
};

struct minmea_sentence_gsv {
    int total_msgs;
    int msg_nr;
    int total_sats;
    struct minmea_sat_info sats[4];
};

/**
 * Check sentence validity and checksum. Returns true for valid sentences.
 */
bool minmea_check(const char *sentence);

/**
 * Determine talker identifier.
 */
bool minmea_talker_id(char talker[3], const char *sentence);

/**
 * Determine sentence identifier.
 */
enum minmea_sentence_id minmea_sentence_id(const char *sentence);

/**
 * Scanf-like processor for NMEA sentences. Supports the following formats:
 * c - single character (char *)
 * d - direction, returned as 1/-1, default 0 (int *)
 * f - fractional, returned as value + scale (int *, int *)
 * i - decimal, default zero (int *)
 * s - string (char *)
 * t - talker identifier and type (char *)
 * T - date/time stamp (int *, int *, int *)
 * Returns true on success. See library source code for details.
 */
bool minmea_scan(const char *sentence, const char *format, ...);

/*
 * Parse a specific type of sentence. Return true on success.
 */
bool minmea_parse_rmc(struct minmea_sentence_rmc *frame, const char *sentence);
bool minmea_parse_gga(struct minmea_sentence_gga *frame, const char *sentence);
bool minmea_parse_gsa(struct minmea_sentence_gsa *frame, const char *sentence);
bool minmea_parse_gst(struct minmea_sentence_gst *frame, const char *sentence);
bool minmea_parse_gsv(struct minmea_sentence_gsv *frame, const char *sentence);

/**
 * Convert GPS UTC date/time representation to a UNIX timestamp.
 */
int minmea_gettimeofday(struct timeval *tv, const struct minmea_date *date, const struct minmea_time *time);

/**
 * Rescale a fixed-point value to a different scale. Rounds towards zero.
 */
static inline int minmea_rescale(int value, int from, int to)
{
    if (from == 0)
        return 0;
    if (from == to)
        return value;
    if (from > to)
        return (value + ((value > 0) - (value < 0)) * from/to/2) / (from/to);
    else
        return value * (to/from);
}

/**
 * Convert a fixed-point value to a floating-point value.
 * Returns NaN for "unknown" values.
 */
static inline float minmea_float(int value, int scale)
{
    if (scale == 0)
        return NAN;
    return (float) value / (float) scale;
}

/**
 * Convert a raw coordinate to a floating point DD.DDD... value.
 * Returns NaN for "unknown" values.
 */
static inline float minmea_coord(int value, int scale)
{
    if (scale == 0)
        return NAN;
    int degrees = value / (scale * 100);
    int minutes = value % (scale * 100);
    return (float) degrees + (float) minutes / (60 * scale);
}

#ifdef __cplusplus
}
#endif

#endif /* MINMEA_H */

/* vim: set ts=4 sw=4 et: */
