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

/**
 * Get latest humidity sensor values
 *
 * @param pressure (out)    Air pressure value in hPa
 * @param temp (out)        Temperature value in Celsius
 * @param humidity (out)    Humidity value
 */
void humidity_sensor_get(float& pressure, float& temp, float& humidity);


#ifdef __cplusplus
}
#endif

#endif // HUMIDITY_SENSOR_H
