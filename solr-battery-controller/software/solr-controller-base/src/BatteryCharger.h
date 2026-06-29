#pragma once

#include "Arduino.h"

#define BATTERY_TYPE \
    /*  ENUM                        MAX_V        MIN_V       RECOVERY_V*/\
    X(LI_ION_3S_BATTERY,         (4.2f * 3), (3.3f * 3), (3.6f * 3)) \
    X(LEAD_ACID_12V_BATTERY,      (13.8f),   (10.5f),    (12.0f))    \
    X(SEALED_LEAD_ACID_BATTERY,   (14.4f),   (10.8f),    (11.5f))    \

typedef enum BatteryType {
    #define X(ENUM, MAX_V, MIN_V, RECOVERY_V) ENUM,
    BATTERY_TYPE
    #undef X
} BatteryType;

typedef enum SlaChargeState {
    SLA_CHARGE_MAIN,
    SLA_CHARGE_HOLD_FLOAT
} SlaChargeState;

typedef struct SlaBatteryCharger {
    SlaChargeState chargeState;
    uint32_t previousCheckMs;
    uint32_t maxVoltageChargeMs;
    uint8_t pwmValue;
} SlaBatteryCharger;

SlaBatteryCharger *handleSlaBatteryCharger(uint8_t mosfetSwitchPin, float batteryVoltage);


const char * getBatteryName(BatteryType battery);
float getBatteryMaxVoltage(BatteryType battery);
float getBatteryMinVoltage(BatteryType battery);
float getBatteryRecoveryVoltage(BatteryType battery);