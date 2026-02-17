#include <Arduino.h>

#include <Sound.h>
#include "ACS712.h"
#include "LM35IC.h"

#define DEBUG_ENABLE_PIN       9
#define BUILD_IN_LED_PIN      13
#define BUZZER_PIN            10
#define INVERTER_SWITCH_PIN   11
#define DC_LOAD_SWITCH_PIN     8
#define COOLING_FAN_PIN        3

#define VOLTAGE_SENSOR_PIN           A0
#define DC_CURRENT_SENSOR_PIN        A1
#define INVERTER_CURRENT_SENSOR_PIN  A2
#define TL494_LOAD_TEMP_SENSOR_PIN   A3
#define EGS002_LOAD_TEMP_SENSOR_PIN  A6

// Arduino NANO has 5.0 volt with a max ADC value of 1023 steps
#define MCU_SUPPLY_VOLTAGE          5.0f
#define ADC_RESOLUTION              1023.0f

#define VOLTAGE_READ_COUNT          4
#define MIN_BATTERY_VOLTAGE         9.9f    // 3.3V * 3S -> 0%
#define BATTERY_RECOVERY_VOLTAGE    10.8f  // 3.6V * 3S -> 10%
#define MAX_ALLOWED_BATTERY_VOLTAGE 12.9f
#define LOW_BATTERY_SIGNAL_COUNT    10

#define CURRENT_READ_CYCLES            50
#define MAX_DC_LOAD_CURRENT            15.0f
#define MAX_INVERTER_PRIMARY_CURRENT   30.0f
#define MAX_TOTAL_LOAD_CURRENT         30.0f
#define CURRENT_OVERLOAD_TIMEOUT_MS    (5 * 1000)
#define MAX_SEQUENTIAL_OVERLOAD_COUNT   4

#define TEMPERATURE_SAMPLING_COUNT      25
#define OVER_TEMPERATURE_TIMEOUT_MS     (5 * 60000)   // 5 minutes
#define TEMPERATURE_READ_INTERVAL_MS    (1 * 1000)    // read temperature every second
#define OVER_TEMPERATURE_SIGNAL_COUNT   10

typedef enum ControllerState{
    STATE_OK = 0,
    STATE_BATTERY_LOW,
    STATE_CURRENT_OVERLOAD,
    STATE_OVER_TEMPERATURE,
    STATE_FATAL_ERROR,  // short circuit in a load etc. Cannot recover
} ControllerState;

typedef enum CoolingFanSpeed {
    COOLING_FAN_OFF = 0,
    COOLING_FAN_LOW = 64,
    COOLING_FAN_MEDIUM = 128,
    COOLING_FAN_HIGH = 255,
} CoolingFanSpeed;

typedef enum TemperatureRange {
    TEMP_RANGE_NORMAL = 0,    // Below 35°C
    TEMP_RANGE_LOW = 35,      // 35-50°C
    TEMP_RANGE_MEDIUM = 50,   // 50-60°C
    TEMP_RANGE_HIGH = 60,     // 60-70°C
    TEMP_RANGE_MAX = 70,      // 70°C and above
} TemperatureRange;

/*
 * Two resistors divider of 30K and 7.5K
 *
 * Vbat     Vin      GND
 *  |        |        |
 *  +--[R1]--+--[R2]--+
 *
 *  R1 = 30kOhms
 *  R2 = 7.5kOhms
*/
static const float R1 = 30000;
static const float R2 = 7500;

static ACS712 dcLoadCurrentSensor(DC_CURRENT_SENSOR_PIN, MCU_SUPPLY_VOLTAGE, ADC_RESOLUTION, 66);            //ACS712 30A uses 66mV per A
static ACS712 inverterCurrentSensor(INVERTER_CURRENT_SENSOR_PIN, MCU_SUPPLY_VOLTAGE, ADC_RESOLUTION, 20);   // ACS758 100A uses 20mV per A

static LM35::LM35IC primaryTemperatureSensor = LM35::LM35IC(TL494_LOAD_TEMP_SENSOR_PIN, LM35::LM35D, 1.0, 0.0, MCU_SUPPLY_VOLTAGE);
static LM35::LM35IC secondaryTemperatureSensor = LM35::LM35IC(EGS002_LOAD_TEMP_SENSOR_PIN, LM35::LM35D, 1.0, 0.0, MCU_SUPPLY_VOLTAGE);

static ControllerState state = STATE_OK;
static float batteryVoltage = 0.0f;
static float dcLoadCurrent = 0.0f;
static float inverterLoadCurrent = 0.0f;
static double primarySideTemp = 0.0;
static double secondarySideTemp = 0.0;

static uint8_t errorSignalCounter = 0;
static uint64_t loadDisableTimeoutMs = 0;
static uint64_t previousTemperatureReadMs = 0;
static uint8_t sequentialOverloadCount = 0;
static uint64_t sequentialOverloadMs = 0;

static float getBatteryVoltage(uint16_t samplingCount);
static float getAcsLoadAmps(ACS712 acs);

static void handleBatteryVoltage();
static void handleDcLoadCurrent();
static void handleInverterLoadCurrent();
static void handleTemperatureSensors();
static void handleControllerState();

static inline bool isDebugEnabled();
static inline void ledToggle();
static inline void inverterSwitchOn();
static inline void inverterSwitchOff();
static inline void dcLoadSwitchOn();
static inline void dcLoadSwitchOff();
static inline void disableAllOutputs();
static inline void setCoolingFanSpeed(CoolingFanSpeed adcValue);
static double getTemperature(LM35::LM35IC sensor);
static inline TemperatureRange getTemperatureRange(double temperature);

void setup() {
    Serial.begin(9600);
    pinMode(BUILD_IN_LED_PIN, OUTPUT);
    pinMode(INVERTER_SWITCH_PIN, OUTPUT);
    pinMode(DC_LOAD_SWITCH_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(COOLING_FAN_PIN, OUTPUT);
    pinMode(DEBUG_ENABLE_PIN, INPUT_PULLUP);
    pinMode(VOLTAGE_SENSOR_PIN, INPUT);
    pinMode(DC_CURRENT_SENSOR_PIN, INPUT);
    pinMode(INVERTER_CURRENT_SENSOR_PIN, INPUT);
    pinMode(TL494_LOAD_TEMP_SENSOR_PIN, INPUT);
    pinMode(EGS002_LOAD_TEMP_SENSOR_PIN, INPUT);

    disableAllOutputs();

    while (!Serial);
    Serial.print("ACS712 lib version: ");
    Serial.println(ACS712_LIB_VERSION);
    delay(100);

    // set mid point for current sensors
    dcLoadCurrentSensor.autoMidPointDC(100);
    dcLoadCurrentSensor.incMidPoint();
    dcLoadCurrentSensor.suppressNoise(true);
    Serial.print("DC Load ACS MidPoint= ");
    Serial.println(dcLoadCurrentSensor.getMidPoint());

    inverterCurrentSensor.autoMidPointDC(100);
    Serial.print("INV Load ACS MidPoint= ");
    Serial.println(inverterCurrentSensor.getMidPoint());

    // stabilize values
    dcLoadCurrentSensor.mA_DC(100);
    inverterCurrentSensor.mA_DC(100);
    getBatteryVoltage(100);

    setCoolingFanSpeed(COOLING_FAN_HIGH);   // run fan at high speed for a moment to check it's working
    delay(100);
    setCoolingFanSpeed(COOLING_FAN_OFF);
    playStartupSound(BUZZER_PIN);
}

void loop() {

    handleBatteryVoltage();
    handleDcLoadCurrent();
    handleInverterLoadCurrent();

    float totalLoadCurrent = dcLoadCurrent + inverterLoadCurrent;
    if (state == STATE_OK && totalLoadCurrent > MAX_TOTAL_LOAD_CURRENT) {
        disableAllOutputs();
        state = STATE_CURRENT_OVERLOAD; // total load too high
        sequentialOverloadCount++;
        loadDisableTimeoutMs = millis();
    }
    handleTemperatureSensors();
    handleControllerState();

    if (isDebugEnabled()) {
        Serial.print("V= ");
        Serial.print(batteryVoltage, 3);
        Serial.print(", DC Amps= ");
        Serial.print(dcLoadCurrent, 3);
        Serial.print(", Inv Amps= ");
        Serial.print(inverterLoadCurrent, 3);
        Serial.print(", Temp1= ");
        Serial.print(primarySideTemp);
        Serial.print("C");
        Serial.print(", Temp2= ");
        Serial.print(secondarySideTemp);
        Serial.print("C");
        Serial.println();
        delay(1000);
        return;
    }
    delay(state == STATE_OK ? 50 : 1000);
}

static void handleBatteryVoltage() {
    if (state == STATE_FATAL_ERROR) return;

    batteryVoltage = getBatteryVoltage(VOLTAGE_READ_COUNT);
    if (batteryVoltage <= MIN_BATTERY_VOLTAGE) {
        state = STATE_BATTERY_LOW;
        disableAllOutputs();

    } else if (batteryVoltage > MAX_ALLOWED_BATTERY_VOLTAGE) {
        disableAllOutputs();
        state = STATE_FATAL_ERROR; // battery voltage too high, something is wrong
        fatalErrorSound(BUZZER_PIN);
    }
}

static void handleDcLoadCurrent() {
    if (state != STATE_OK) return;

    dcLoadSwitchOn();
    dcLoadCurrent = getAcsLoadAmps(dcLoadCurrentSensor);
    if (dcLoadCurrent > MAX_DC_LOAD_CURRENT) {
        disableAllOutputs();
        state = STATE_CURRENT_OVERLOAD;
        sequentialOverloadCount++;
        loadDisableTimeoutMs = millis();
    }
}

static void handleInverterLoadCurrent() {
    if (state != STATE_OK) return;

    inverterSwitchOn();
    inverterLoadCurrent = getAcsLoadAmps(inverterCurrentSensor);
    if (inverterLoadCurrent > MAX_INVERTER_PRIMARY_CURRENT) {
        disableAllOutputs();
        state = STATE_CURRENT_OVERLOAD;
        sequentialOverloadCount++;
        loadDisableTimeoutMs = millis();
    }
}

static void handleTemperatureSensors() {
    if (state != STATE_OK) return;

    uint64_t currentMillis = millis();
    if ((currentMillis - previousTemperatureReadMs) >= TEMPERATURE_READ_INTERVAL_MS) {
        primarySideTemp = getTemperature(primaryTemperatureSensor);
        secondarySideTemp = getTemperature(secondaryTemperatureSensor);

        TemperatureRange primaryTempRange = getTemperatureRange(primarySideTemp);
        TemperatureRange secondaryTempRange = getTemperatureRange(secondarySideTemp);
        if (primaryTempRange == TEMP_RANGE_HIGH || secondaryTempRange == TEMP_RANGE_HIGH) {
            setCoolingFanSpeed(COOLING_FAN_HIGH);

        } else if (primaryTempRange == TEMP_RANGE_MEDIUM || secondaryTempRange == TEMP_RANGE_MEDIUM) {
            setCoolingFanSpeed(COOLING_FAN_MEDIUM);

        } else if (primaryTempRange == TEMP_RANGE_LOW || secondaryTempRange == TEMP_RANGE_LOW) {
            setCoolingFanSpeed(COOLING_FAN_LOW);

        } else if (primaryTempRange == TEMP_RANGE_MAX || secondaryTempRange == TEMP_RANGE_MAX) {
            disableAllOutputs();
            setCoolingFanSpeed(COOLING_FAN_HIGH);
            state = STATE_OVER_TEMPERATURE; // temperature too high
            loadDisableTimeoutMs = currentMillis;

        } else {
            setCoolingFanSpeed(COOLING_FAN_OFF);
        }
        previousTemperatureReadMs = currentMillis;
    }
}

static void handleControllerState() {
    if (state == STATE_OK || state == STATE_FATAL_ERROR) return;

    uint64_t currentMillis = millis();
    if (state == STATE_BATTERY_LOW && (batteryVoltage < BATTERY_RECOVERY_VOLTAGE)) { // battery charge should be at least 10% to recover
        if (errorSignalCounter < LOW_BATTERY_SIGNAL_COUNT) {
            lowBatterySound(BUZZER_PIN);
            errorSignalCounter++;
        }
        ledToggle();
        return;    // battery is not charged yet, wait some time and return
    }

    if (state == STATE_CURRENT_OVERLOAD && ((currentMillis - loadDisableTimeoutMs) < CURRENT_OVERLOAD_TIMEOUT_MS)) {
        currentTooHighSound(BUZZER_PIN);
        ledToggle();

        if ((currentMillis - sequentialOverloadMs) > (CURRENT_OVERLOAD_TIMEOUT_MS * MAX_SEQUENTIAL_OVERLOAD_COUNT)) {
            sequentialOverloadCount = 0;    // reset counter if overloads are not in time frame
            sequentialOverloadMs = 0;
        }

        if (sequentialOverloadCount >= MAX_SEQUENTIAL_OVERLOAD_COUNT) {
            fatalErrorSound(BUZZER_PIN);
            state = STATE_FATAL_ERROR; // too many sequential overloads, looks like a short circuit
            return;
        }
        sequentialOverloadMs = currentMillis;
        return;
    }

    if (state == STATE_OVER_TEMPERATURE && ((currentMillis - loadDisableTimeoutMs) < OVER_TEMPERATURE_TIMEOUT_MS)) {
        if (errorSignalCounter < OVER_TEMPERATURE_SIGNAL_COUNT) {
            overTemperatureSound(BUZZER_PIN);
            errorSignalCounter++;
        }
        ledToggle();
        return;
    }

    state = STATE_OK;
    systemUpSound(BUZZER_PIN);
    loadDisableTimeoutMs = 0;
    errorSignalCounter = 0;
}

static inline bool isDebugEnabled() {
    return digitalRead(DEBUG_ENABLE_PIN) == LOW;     // check that debug jumper is set to GND
}

static float getBatteryVoltage(uint16_t samplingCount) {
    float accumulatedValue = 0;
    for (uint16_t i = 0; i < samplingCount; i++) {
        float adcValue = analogRead(VOLTAGE_SENSOR_PIN);
        float outVoltage = (adcValue * MCU_SUPPLY_VOLTAGE) / ADC_RESOLUTION;
        float dividerVoltage = outVoltage / (R2 / (R1 + R2));
        accumulatedValue += dividerVoltage;
        delay(10);
    }
    return accumulatedValue / samplingCount;
}

static float getAcsLoadAmps(ACS712 acs) {
    float amps = (acs.mA_DC(CURRENT_READ_CYCLES) / 1000);
    return amps > 0 ? amps : 0.0f;
}

static inline void ledToggle() {
    digitalWrite(BUILD_IN_LED_PIN, !digitalRead(BUILD_IN_LED_PIN));
}

static inline void inverterSwitchOn() {
    digitalWrite(INVERTER_SWITCH_PIN, HIGH);
}

static inline void inverterSwitchOff() {
    digitalWrite(INVERTER_SWITCH_PIN, LOW);
}

static inline void dcLoadSwitchOn() {
    digitalWrite(DC_LOAD_SWITCH_PIN, HIGH);
    digitalWrite(BUILD_IN_LED_PIN, HIGH);
}

static inline void dcLoadSwitchOff() {
    digitalWrite(DC_LOAD_SWITCH_PIN, LOW);
    digitalWrite(BUILD_IN_LED_PIN, LOW);
}

static inline void disableAllOutputs() {
    inverterSwitchOff();
    dcLoadSwitchOff();
    setCoolingFanSpeed(COOLING_FAN_OFF);
}

static double getTemperature(LM35::LM35IC sensor) {
    double temperature = 0.0;
    for (uint8_t i = 0; i < TEMPERATURE_SAMPLING_COUNT; i++) {
        temperature += sensor.readTemp();
        delay(10);
    }
    return temperature / TEMPERATURE_SAMPLING_COUNT;
}

static inline TemperatureRange getTemperatureRange(double temperature) {
    if (temperature >= TEMP_RANGE_MAX) return TEMP_RANGE_MAX;
    if (temperature >= TEMP_RANGE_HIGH) return TEMP_RANGE_HIGH;
    if (temperature >= TEMP_RANGE_MEDIUM) return TEMP_RANGE_MEDIUM;
    if (temperature >= TEMP_RANGE_LOW) return TEMP_RANGE_LOW;
    return TEMP_RANGE_NORMAL;
}

static inline void setCoolingFanSpeed(CoolingFanSpeed adcValue) {
    analogWrite(COOLING_FAN_PIN, adcValue);
}
