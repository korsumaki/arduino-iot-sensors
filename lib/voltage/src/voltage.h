// Supply voltage measurement

#ifndef VOLTAGE_H
#define VOLTAGE_H

#ifdef __cplusplus
extern "C" {
#endif


extern int adc_value; // TODO temporary global variable
extern float adc_voltage; // TODO temporary global variable


/**
 * Start voltage measurement
 */
void voltage_measure(void);

/**
 * Get latest voltage
 */
float voltage_get(void);


#ifdef __cplusplus
}
#endif

#endif // VOLTAGE_H
