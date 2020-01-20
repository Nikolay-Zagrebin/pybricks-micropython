// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2019 Laurens Valk
// Copyright (c) 2019 LEGO System A/S

#ifndef _PBIO_TRAJECTORY_H_
#define _PBIO_TRAJECTORY_H_

#include <stdint.h>
#include <stdio.h>

#include <pbdrv/config.h>

#include <pbio/error.h>
#include <pbio/port.h>

#define MS_PER_SECOND (1000)
#define US_PER_MS (1000)
#define US_PER_SECOND (1000000)

#define to_mcount(count) (((int64_t) (count))*1000)
#define to_count(mcount) ((int32_t)((mcount)/1000))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

// Macro to evaluate b*t/US_PER_SECOND in two steps to avoid excessive round-off errors and overflows.
#define timest(b, t) ((b * ((t)/US_PER_MS))/MS_PER_SECOND)
// Same trick to evaluate formulas of the form 1/2*b*t^2/US_PER_SECOND^2
#define timest2(b, t) ((timest(timest(b, (t)),(t)))/2)
// Macro to evaluate division of speed by acceleration (w/a), yielding time, in the appropriate units
#define wdiva(w, a) ((((w)*US_PER_MS)/a)*MS_PER_SECOND)

/**
 * Motor trajectory parameters for an ideal maneuver without disturbances
 */
typedef struct _pbio_trajectory_t {
    bool forever;                       /**<  Whether maneuver has end-point */
    int32_t t0;                        /**<  Time at start of maneuver */
    int32_t t1;                        /**<  Time after the acceleration in-phase */
    int32_t t2;                        /**<  Time at start of acceleration out-phase */
    int32_t t3;                        /**<  Time at end of maneuver */
    int32_t th0;                        /**<  Encoder count at start of maneuver */
    int32_t th1;                        /**<  Encoder count after the acceleration in-phase */
    int32_t th2;                        /**<  Encoder count at start of acceleration out-phase */
    int32_t th3;                        /**<  Encoder count at end of maneuver */
    int64_t mth0;                        /**<  As above, but millicounts */
    int64_t mth1;                        /**<  As above, but millicounts */
    int64_t mth2;                        /**<  As above, but millicounts */
    int64_t mth3;                        /**<  As above, but millicounts */
    int32_t w0;                          /**<  Encoder rate at start of maneuver */
    int32_t w1;                          /**<  Encoder rate target when not accelerating */
    int32_t a0;                          /**<  Encoder acceleration during in-phase */
    int32_t a2;                          /**<  Encoder acceleration during out-phase */
} pbio_trajectory_t;

// Core trajectory generators

void pbio_trajectory_make_stationary(pbio_trajectory_t *ref, int32_t t0, int32_t th0, int32_t w1);

pbio_error_t pbio_trajectory_make_time_based(pbio_trajectory_t *ref, bool forever, int32_t t0, int32_t t3, int32_t th0, int32_t th0_ext, int32_t w0, int32_t wt, int32_t wmax, int32_t a);

pbio_error_t pbio_trajectory_make_angle_based(pbio_trajectory_t *ref, int32_t t0, int32_t th0, int32_t th3, int32_t w0, int32_t wt, int32_t wmax, int32_t a);

void pbio_trajectory_get_reference(pbio_trajectory_t *traject, int32_t time_ref, int32_t *count_ref, int32_t *count_ref_ext, int32_t *rate_ref, int32_t *acceleration_ref);

// Extended and patched trajectories

void pbio_trajectory_get_start(pbio_trajectory_t *traject, int8_t segment, int32_t *start_count, int32_t *start_count_ext);

pbio_error_t pbio_trajectory_make_time_based_patched(pbio_trajectory_t *ref, bool forever, int32_t t0, int32_t t3, int32_t wt, int32_t wmax, int32_t a);

#endif // _PBIO_TRAJECTORY_H_
