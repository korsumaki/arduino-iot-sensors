#include "value_analysis.h"

void value_analysis_init(void)
{
    // TODO is this needed?
}

void value_analysis_add_value(value_analysis_t *analysis, float value, int current_time)
{
    // Calculate changes
    if (analysis->latest_time_ms > 0)
    {
        analysis->latest_duration_ms = current_time - analysis->latest_time_ms;
        analysis->value_delta = value - analysis->latest_value; // <- Change is positive when value is increasing
    }

    // Store time and value
    analysis->latest_time_ms = current_time;
    analysis->latest_value = value;
    ;
}

float value_analysis_get_change(value_analysis_t *analysis, int time_span_ms)
{
    // It is meaningful to calculate change only when we know how long duration it was
    if (analysis->latest_duration_ms > 0)
    {
        return analysis->value_delta * time_span_ms / analysis->latest_duration_ms;
    }
    return 0.0f;
}