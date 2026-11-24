#include "temperature_sensor.h"
#include "scheduler.h"

#include <OneWire.h>
#include <DallasTemperature.h>


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

static float tempC = DEVICE_DISCONNECTED_C;

float temperature_get(void)
{
    return tempC;
}

static int temperature_sensor_meas_task(void)
{
    static bool is_temperature_requested = false;
    // Give 10ms more time for conversion
    static int conversion_time = sensors.millisToWaitForConversion() + 10;

    int time_for_next_call = 0;

    if (!is_temperature_requested)
    {
        sensors.requestTemperatures(); // Send the command to get temperatures
        is_temperature_requested = true;
        time_for_next_call = conversion_time;
    }
    else
    {
        is_temperature_requested = false;
        tempC = sensors.getTempCByIndex(0);

        // Check if reading was successful
        if (tempC != DEVICE_DISCONNECTED_C)
        {
            Serial.print("Temperature: ");
            Serial.println(tempC);
        }
        else
        {
            Serial.println("Error: Could not read temperature data");
        }
        time_for_next_call = SCHEDULER_STOP_TASK;
    }

    return time_for_next_call;
}

void temperature_sensor_init(void)
{
    // Start up the library
    sensors.begin();
}

void temperature_sensor_measure(void)
{
    // Add task to scheduler, to start temperature measurement
    scheduler_add_task(temperature_sensor_meas_task, 0);
}
