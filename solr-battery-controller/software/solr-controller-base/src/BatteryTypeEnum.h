#pragma once

extern "C" {

#define MAX_BATTERY_LOAD_CURRENT_AMPS 30

#define BATTERY_TYPE \
    /*  ENUM                MIN_V        MAX_V       RECOV_V - 10% of charge*/         \
    X(LI_ION_3S,         (4.2f * 3), (3.3f * 3), (3.6f * 3)) \
    X(LI_ION_2S,         (4.2f * 2), (3.3f * 2), (3.6f * 2)) \
    X(LEAD_ACID_12V,      (13.8f),   (10.5f),    (12.0f))    \
    X(SEALED_LEAD_ACID,   (14.4f),   (10.5f),    (11.5f))    \

typedef enum BatteryType {
    #define X(ENUM, MIN_V, MAX_V, RECOV_V) ENUM,
    BATTERY_TYPE
    #undef X
} BatteryType;

const char * getBatteryName(BatteryType battery) {
    switch (battery) {
        #define X(ENUM, MIN_V, MAX_V, RECOV_V) \
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
        #define X(ENUM, MIN_V, MAX_V, RECOV_V) \
            case ENUM:  \
                return MIN_V;
        BATTERY_TYPE
            default:
                return 0.0;
        #undef X
    }
}

float getBatteryMinVoltage(BatteryType battery) {
    switch (battery) {
        #define X(ENUM, MIN_V, MAX_V, RECOV_V) \
            case ENUM:  \
                return MAX_V;
        BATTERY_TYPE
            default:
                return 0.0;
        #undef X
    }
}

float getBatteryRecoveryVoltage(BatteryType battery) {
    switch (battery) {
        #define X(ENUM, MIN_V, MAX_V, RECOV_V) \
            case ENUM:  \
                return RECOV_V;
        BATTERY_TYPE
            default:
                return 0.0;
        #undef X
    }
}

} // extern "C"