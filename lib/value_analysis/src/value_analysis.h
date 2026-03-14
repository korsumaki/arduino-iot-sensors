// Value analysis library

#ifndef VALUE_ANALYSIS_H
#define VALUE_ANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float latest_value;
    float value_delta; //_in_duration;
    int latest_time_ms;
    int latest_duration_ms;
    int time_period_sec;
} value_analysis_t;

/**
 * Initialize analysis instance
 *
 * @param analysis          analysis instance
 * @param time_period_sec   time period for analysis (in seconds)
 */
void value_analysis_init(value_analysis_t *analysis, int time_period_sec);

/**
 * Add new measurement for analysis
 * 
 * @param analysis          analysis instance
 * @param value             current measured value
 * @param current_time_ms   actual time during latest measurement (milliseconds)
 * @return                  analyzed value
 */
float value_analysis_add_value(value_analysis_t *analysis, float value, int current_time_ms);

/**
 * Get latest analyzed value
 * 
 * @param analysis          analysis instance
 * @param time_period_sec   time period for analysis (in seconds), this can be different than initialized
 * @return                  analyzed value
 */
float value_analysis_get_change(value_analysis_t *analysis, int time_period_sec);

#ifdef __cplusplus
}
#endif

#endif // VALUE_ANALYSIS_H
