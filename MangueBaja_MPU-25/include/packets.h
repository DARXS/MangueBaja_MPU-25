#ifndef PACKETS_H
#define PACKETS_H

#include <stdio.h>
#include <string.h>

#define SUCESS_RESPONSE 2
#define FAIL_RESPONSE   1

typedef struct
{
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
} imu_acc_t;

typedef struct
{
    int16_t dps_x;
    int16_t dps_y;
    int16_t dps_z;
} imu_dps_t;

typedef struct
{
    //int cont;
    /* Mangue Telemetry Struct */
    imu_acc_t imu_acc;
    imu_dps_t imu_dps;
    uint16_t rpm;
    uint16_t speed;
    uint8_t temperature;
    uint8_t flags; // MSB - BOX | BUFFER FULL | NC | NC | FUEL_LEVEL | SERVO_ERROR | CHK | RUN - LSB
    uint8_t SOC;
    uint8_t cvt;
    float volt;
    double latitude;
    double longitude;
    //uint16_t fuel_level;
    uint32_t timestamp;
    uint8_t sat;
} radio_packet_t;

typedef struct
{
    //MPU_Bluetooth (sent by physical serial connection)
    // String config_bluetooth_enabled;
    // String config_bluedroid_enabled;
    // String config_bt_spp_enabled;

    //MPU
    uint8_t lora_init;

    //SCU
    uint8_t internet_modem;
    uint8_t mqtt_client_connection;
    uint8_t sd_start;
    uint8_t check_sd;

    //FRONT
    uint8_t accel_begin;
    
    //REAR
    uint8_t termistor;
    uint8_t cvt_temperature;
    uint8_t measure_volt;
    uint8_t speed_pulse_counter;
    uint16_t servo_state;
    
} bluetooth;

#endif