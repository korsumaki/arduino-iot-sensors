// Temperature sensor DS18B20

#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

// Data wire pin
#define ONE_WIRE_BUS  D6

/**
 * Initialize temperature sensor driver
 */
void temperature_sensor_init(void);

/**
 * Start temperature sensor measurement
 */
void temperature_sensor_measure(void);


#ifdef __cplusplus
}
#endif

#endif // TEMPERATURE_SENSOR_H
