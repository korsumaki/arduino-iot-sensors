// Humidity sensor BME280

#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize hunidity sensor driver
 */
void humidity_sensor_init(void);

/**
 * Start humidity sensor measurement
 */
void humidity_sensor_measure(void);


#ifdef __cplusplus
}
#endif

#endif // HUMIDITY_SENSOR_H
