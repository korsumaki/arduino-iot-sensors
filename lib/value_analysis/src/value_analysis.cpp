#include "value_analysis.h"

void value_analysis_init(value_analysis_t *analysis, int time_period_sec)
{
    analysis->latest_duration_ms = 0;
    analysis->latest_time_ms = 0;
    analysis->latest_value = 0.0f;
    analysis->time_period_sec = time_period_sec;
    analysis->value_delta = 0.0f;
}

float value_analysis_add_value(value_analysis_t *analysis, float value, int current_time_ms)
{
    float value_change = 0.0f;
    // Calculate changes
    if (analysis->latest_time_ms > 0)
    {
        analysis->latest_duration_ms = current_time_ms - analysis->latest_time_ms;
        analysis->value_delta = value - analysis->latest_value; // <- Change is positive when value is increasing

        value_change = analysis->value_delta * (analysis->time_period_sec*1000) / analysis->latest_duration_ms;
    }

    // Store time and value
    analysis->latest_time_ms = current_time_ms;
    analysis->latest_value = value;

    return value_change;
}

float value_analysis_get_change(value_analysis_t *analysis, int time_period_sec)
{
    // It is meaningful to calculate change only when we know how long duration it was
    if (analysis->latest_duration_ms > 0)
    {
        return analysis->value_delta * (time_period_sec*1000) / analysis->latest_duration_ms;
    }
    return 0.0f;
}