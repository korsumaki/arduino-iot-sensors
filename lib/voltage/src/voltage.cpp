#include <Arduino.h>
#include "voltage.h"
#include "scheduler.h"


// MKO-01 rev0
#define VOLTAGE_MEAS_UPPER_RESISITOR (470.0f) // 470k
#define VOLTAGE_MEAS_LOWER_RESISITOR (100.0f) // 100k
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
    const float V_REF = 3.3;     // Analog reference voltage (e.g., 5V or 3.3V)
    const float R_BITS = 10.0;   // ADC resolution (bits)
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
