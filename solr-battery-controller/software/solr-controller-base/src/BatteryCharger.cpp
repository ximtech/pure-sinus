#include "BatteryCharger.h"

#define CHARGER_DELAY_MS 50

#define SLA_FLOATING_VOLTAGE  13.7f
#define SLA_RECONNECT_VOLTAGE 12.6

#define ONE_HOUR_DELAY (1UL * 60 * 60 * 1000)

static SlaBatteryCharger slaBattery = {};

SlaBatteryCharger *handleSlaBatteryCharger(uint8_t mosfetSwitchPin, float batteryVoltage) {
    if (millis() - slaBattery.previousCheckMs < CHARGER_DELAY_MS) {
        return &slaBattery;
    }

    if (slaBattery.chargeState == SLA_CHARGE_MAIN) {
        slaBattery.pwmValue = 255;

        // when battery fully charged, starting 1-hour timer and wait until battery fully saturated
        if (batteryVoltage >= getBatteryMaxVoltage(SEALED_LEAD_ACID_BATTERY)) {
            if (slaBattery.maxVoltageChargeMs == 0) {
                slaBattery.maxVoltageChargeMs = millis();
            }

            if ((millis() - slaBattery.maxVoltageChargeMs) > ONE_HOUR_DELAY) {
                slaBattery.chargeState = SLA_CHARGE_HOLD_FLOAT;
            }
        } else {
            slaBattery.maxVoltageChargeMs = 0;  // voltage dropped, reset timer
        }


    } else if (slaBattery.chargeState == SLA_CHARGE_HOLD_FLOAT) {   // holding voltage at 13.7V, changing PWM value

        if (batteryVoltage < SLA_FLOATING_VOLTAGE && slaBattery.pwmValue < 255) {
            slaBattery.pwmValue++;
        }

        if (batteryVoltage > SLA_FLOATING_VOLTAGE && slaBattery.pwmValue > 0) {
            slaBattery.pwmValue--;
        }

        if (batteryVoltage < SLA_RECONNECT_VOLTAGE) {
            slaBattery.chargeState = SLA_CHARGE_MAIN;
            slaBattery.maxVoltageChargeMs = 0;
        }
    }

    analogWrite(mosfetSwitchPin, slaBattery.pwmValue);
    slaBattery.previousCheckMs = millis();
    return &slaBattery;
}

const char * getBatteryName(BatteryType battery) {
    switch (battery) {
        #define X(ENUM, MAX_V, MIN_V, RECOVERY_V) \
            case ENUM:  \
                return #ENUM;
        BATTERY_TYPE
        default:
            return "UNKNOWN";
            #undef X
    }
}

float getBatteryMaxVoltage(BatteryType battery) {
    switch (battery) {
        #define X(ENUM, MAX_V, MIN_V, RECOVERY_V) \
            case ENUM:  \
                return MAX_V;
        BATTERY_TYPE
        default:
            return 0.0;
            #undef X
    }
}

float getBatteryMinVoltage(BatteryType battery) {
    switch (battery) {
        #define X(ENUM, MAX_V, MIN_V, RECOVERY_V) \
            case ENUM:  \
                return MIN_V;
        BATTERY_TYPE
        default:
            return 0.0;
            #undef X
    }
}

float getBatteryRecoveryVoltage(BatteryType battery) {
    switch (battery) {
        #define X(ENUM, MAX_V, MIN_V, RECOVERY_V) \
            case ENUM:  \
                return RECOVERY_V;
        BATTERY_TYPE
        default:
            return 0.0;
            #undef X
    }
}