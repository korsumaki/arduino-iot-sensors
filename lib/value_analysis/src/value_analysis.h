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
} value_analysis_t;

/**
 * Initialize ????
 */
void value_analysis_init(void);

/**
 * Add new measurement for analysis
 * 
 * @param analysis      analysis instance
 * @param value         current measured value
 * @param current_time  actual time during latest measurement
 */
void value_analysis_add_value(value_analysis_t *analysis, float value, int current_time);

/**
 * Add new measurement for analysis
 * 
 * @param analysis      analysis instance
 * @param time_span_ms  time which change is wanted
 * @return 
 */
float value_analysis_get_change(value_analysis_t *analysis, int time_span_ms);

#ifdef __cplusplus
}
#endif

#endif // VALUE_ANALYSIS_H
