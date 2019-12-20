// SPDX-License-Identifier: MIT
// Copyright (c) 2019 Laurens Valk

#include <math.h>

#include <pbio/servo.h>
#include <pbio/drivebase.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "py/obj.h"

#include "pberror.h"
#include "pbobj.h"
#include "pbkwarg.h"
#include "modmotor.h"
#include "pbthread.h"

#if PYBRICKS_PY_ROBOTICS

// pybricks.robotics.DriveBase class object
typedef struct _robotics_DriveBase_obj_t {
    mp_obj_base_t base;
    pbio_drivebase_t *db;
    motor_Motor_obj_t *left;
    motor_Motor_obj_t *right;
} robotics_DriveBase_obj_t;

// pybricks.robotics.DriveBase.__init__
STATIC mp_obj_t robotics_DriveBase_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args ) {

    PB_PARSE_ARGS_CLASS(n_args, n_kw, args,
        PB_ARG_REQUIRED(left_motor),
        PB_ARG_REQUIRED(right_motor),
        PB_ARG_REQUIRED(wheel_diameter),
        PB_ARG_REQUIRED(axle_track)
    );

    robotics_DriveBase_obj_t *self = m_new_obj(robotics_DriveBase_obj_t);
    self->base.type = (mp_obj_type_t*) type;

    // Argument must be two motors and two dimensions
    if (!MP_OBJ_IS_TYPE(left_motor, &motor_Motor_type) || !MP_OBJ_IS_TYPE(right_motor, &motor_Motor_type)) {
        pb_assert(PBIO_ERROR_INVALID_ARG);
    }
    
    // Pointer to the Python (not pbio) Motor objects
    self->left = MP_OBJ_TO_PTR(left_motor);
    self->right = MP_OBJ_TO_PTR(right_motor);

    // Get wheel diameter and axle track dimensions
    fix16_t wheel_diameter_val = pb_obj_get_fix16(wheel_diameter);
    fix16_t axle_track_val = pb_obj_get_fix16(axle_track);

    // Create drivebase
    pb_thread_enter();
    pbio_error_t err = pbio_drivebase_get(&self->db, self->left->srv, self->right->srv, wheel_diameter_val, axle_track_val);
    pb_thread_exit();
    pb_assert(err);

    return MP_OBJ_FROM_PTR(self);
}

// pybricks.robotics.DriveBase.start
STATIC mp_obj_t robotics_DriveBase_start(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        PB_ARG_REQUIRED(speed),
        PB_ARG_REQUIRED(turn_rate)
    );
    robotics_DriveBase_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    // Get wheel diameter and axle track dimensions
    int32_t speed_val = pb_obj_get_int(speed);
    int32_t turn_rate_val = pb_obj_get_int(turn_rate);

    pbio_error_t err;
    pb_thread_enter();    
    err = pbio_drivebase_start(self->db, speed_val, turn_rate_val);
    pb_thread_exit();
    pb_assert(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(robotics_DriveBase_start_obj, 0, robotics_DriveBase_start);

// LEGACY METHODS AVAILABLE ON EV3 ONLY
#if PYBRICKS_PY_EV3DEVICES 

// pybricks.robotics.DriveBase.drive (Legacy function for 1.0 API compatibility)
STATIC mp_obj_t robotics_DriveBase_drive(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        PB_ARG_REQUIRED(speed),
        PB_ARG_REQUIRED(steering)
    );
    robotics_DriveBase_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    // Get wheel diameter and axle track dimensions
    int32_t speed_val = pb_obj_get_int(speed);
    int32_t turn_rate_val = pb_obj_get_int(steering);

    pbio_error_t err;
    pb_thread_enter();    
    err = pbio_drivebase_start(self->db, speed_val, turn_rate_val);
    pb_thread_exit();
    pb_assert(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(robotics_DriveBase_drive_obj, 0, robotics_DriveBase_drive);

// pybricks.robotics.DriveBase.drive_time (Legacy function for 1.0 API compatibility)
STATIC mp_obj_t robotics_DriveBase_drive_time(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        PB_ARG_REQUIRED(speed),
        PB_ARG_REQUIRED(steering),
        PB_ARG_REQUIRED(time)
    );
    robotics_DriveBase_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    // Get wheel diameter and axle track dimensions
    int32_t speed_val = pb_obj_get_int(speed);
    int32_t turn_rate_val = pb_obj_get_int(steering);
    mp_int_t duration = pb_obj_get_int(time);
    if (duration < 0) {
        pb_assert(PBIO_ERROR_INVALID_ARG);
    }

    pbio_error_t err;
    pb_thread_enter();    
    err = pbio_drivebase_start(self->db, speed_val, turn_rate_val);
    pb_thread_exit();
    pb_assert(err);

    mp_hal_delay_ms(duration);

    pb_thread_enter();    
    err = pbio_drivebase_stop(self->db, PBIO_ACTUATION_COAST);
    pb_thread_exit();
    pb_assert(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(robotics_DriveBase_drive_time_obj, 0, robotics_DriveBase_drive_time);

#endif // LEGACY METHODS AVAILABLE ON EV3 ONLY

// pybricks.robotics.DriveBase.stop
STATIC mp_obj_t robotics_DriveBase_stop(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        PB_ARG_DEFAULT_INT(stop_type, PBIO_ACTUATION_COAST)
    );
    robotics_DriveBase_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    pbio_actuation_t after_stop = mp_obj_get_int(stop_type);

    pbio_error_t err;
    pb_thread_enter();    
    err = pbio_drivebase_stop(self->db, after_stop);
    pb_thread_exit();
    pb_assert(err);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(robotics_DriveBase_stop_obj, 0, robotics_DriveBase_stop);

// dir(pybricks.robotics.DriveBase)
STATIC const mp_rom_map_elem_t robotics_DriveBase_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_left), MP_ROM_ATTRIBUTE_OFFSET(robotics_DriveBase_obj_t, left) },
    { MP_ROM_QSTR(MP_QSTR_right), MP_ROM_ATTRIBUTE_OFFSET(robotics_DriveBase_obj_t, right) },
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&robotics_DriveBase_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&robotics_DriveBase_stop_obj) },
#if PYBRICKS_PY_EV3DEVICES // LEGACY METHODS AVAILABLE ON EV3 ONLY
    { MP_ROM_QSTR(MP_QSTR_drive), MP_ROM_PTR(&robotics_DriveBase_drive_obj) },
    { MP_ROM_QSTR(MP_QSTR_drive_time), MP_ROM_PTR(&robotics_DriveBase_drive_time_obj) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(robotics_DriveBase_locals_dict, robotics_DriveBase_locals_dict_table);

// type(pybricks.robotics.DriveBase)
STATIC const mp_obj_type_t robotics_DriveBase_type = {
    { &mp_type_type },
    .name = MP_QSTR_DriveBase,
    .make_new = robotics_DriveBase_make_new,
    .locals_dict = (mp_obj_dict_t*)&robotics_DriveBase_locals_dict,
};

// dir(pybricks.robotics)
STATIC const mp_rom_map_elem_t robotics_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_robotics)         },
    { MP_ROM_QSTR(MP_QSTR_DriveBase),   MP_ROM_PTR(&robotics_DriveBase_type)  },
};
STATIC MP_DEFINE_CONST_DICT(pb_module_robotics_globals, robotics_globals_table);

const mp_obj_module_t pb_module_robotics = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&pb_module_robotics_globals,
};

#endif // PYBRICKS_PY_ROBOTICS
