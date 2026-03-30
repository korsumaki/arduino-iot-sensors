#include <Arduino.h>
#include "voltage.h"
#include "scheduler.h"


// MKO-01 rev0
#define RES_MKO_01_UPPER_RESISITOR (470.0f) // 470k
#define RES_MKO_01_LOWER_RESISITOR (100.0f) // 100k (parallel with Wemos adc internal resistors)

// Wemos d1 mini (internal adc resistors)
#define RES_WEMOS_ADC_RESISTOR (220.0f+100.0f) // 220k + 100k

// Combined lower resistor: MKO-01 & Wemos parallel resistors
#define RES_COMBINED_LOWER_RESISITOR (1.0f / ((1.0f/RES_WEMOS_ADC_RESISTOR) + (1.0f/RES_MKO_01_LOWER_RESISITOR)))

#define VOLTAGE_MEAS_UPPER_RESISITOR    RES_MKO_01_UPPER_RESISITOR
#define VOLTAGE_MEAS_LOWER_RESISITOR    RES_COMBINED_LOWER_RESISITOR
#define VOLTAGE_MEAS_ADC_PIN         (A0)


int adc_value = 0; // TODO temporary
float adc_voltage = NAN; // TODO temporary

static float supply_voltage = NAN;


float voltage_get(void)
{
    return supply_voltage;
}

static int voltage_meas_task(void)
{
    // Constants
    const float V_REF = 3.2f;   // Analog reference voltage (e.g., 5V or 3.3V)
    const float R_BITS = 10;    // ADC resolution (bits)
    const float ADC_STEPS = (1 << int(R_BITS)) - 1; // Number of steps (2^R_BITS - 1)

    /*int*/ adc_value = analogRead(VOLTAGE_MEAS_ADC_PIN);
    /*float*/ adc_voltage = (adc_value / ADC_STEPS) * V_REF; // Convert to voltage

    supply_voltage = adc_voltage * (VOLTAGE_MEAS_UPPER_RESISITOR+VOLTAGE_MEAS_LOWER_RESISITOR) / VOLTAGE_MEAS_LOWER_RESISITOR;

    Serial.printf("adc=%d, voltage=%f, supply_voltage=%f\n", adc_value, adc_voltage, supply_voltage);

    return SCHEDULER_STOP_TASK;
}


void voltage_measure(void)
{
    // Add task to scheduler
    scheduler_add_task(voltage_meas_task, 0);
}
