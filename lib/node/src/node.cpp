#include <Arduino.h>
#include <Meshtastic.h>
#include <node.h>
#include <scheduler.h>


// Pins to use for SoftwareSerial. Boards that don't use SoftwareSerial, and
// instead provide their own Serial1 connection through fixed pins
// will ignore these settings and use their own.
#define SERIAL_RX_PIN 13
#define SERIAL_TX_PIN 15

// A different baud rate to communicate with the Meshtastic device can be specified here
#define BAUD_RATE 57600

static bool not_yet_connected = true;

static uint32_t received_count = 0;
static uint32_t sent_count = 0;

static void connected_callback(mt_node_t *nodeinfo, mt_nr_progress_t progress);
static void text_message_callback(uint32_t from, uint32_t to, uint8_t channel, const char *text);


void node_init(void)
{
    Serial.println("Booted Meshtastic send/receive client in serial mode");
    mt_serial_init(SERIAL_RX_PIN, SERIAL_TX_PIN, BAUD_RATE);

    // Set to true if you want debug messages
    mt_set_debug(false);

    randomSeed(micros());

    // Initial connection to the Meshtastic device
    mt_request_node_report(connected_callback);

    // Register a callback function to be called whenever a text message is received
    set_text_message_callback(text_message_callback);
}

bool node_is_connected(void)
{
    return !not_yet_connected;
}

uint32_t node_get_received_count(void)
{
    return received_count;
}

uint32_t node_get_sent_count(void)
{
    return sent_count;
}

void node_loop(void)
{
    // Record the time that this loop began (in milliseconds since the device booted)
    uint32_t now = millis();

    // Run the Meshtastic loop, and see if it's able to send requests to the device yet
    /*bool can_send =*/ (void) mt_loop(now);
}

bool node_send_message(uint8_t channel_index, const char *msg)
{
    assert(msg != NULL);
    assert(channel_index < 8);

    bool is_send_successful = false;

    if (node_is_connected())
    {
        sent_count++;

        // Change this to a specific node number if you want to send to just one node
        uint32_t dest = BROADCAST_ADDR; 

        is_send_successful = mt_send_text(msg, dest, channel_index);
    }
    return is_send_successful;
}


// This callback function will be called whenever the radio connects to a node
static void connected_callback(mt_node_t *nodeinfo, mt_nr_progress_t progress)
{
    if (not_yet_connected)
    {
        Serial.println("Connected to Meshtastic device!");
    }
    not_yet_connected = false;
}

// This callback function will be called whenever the radio receives a text message
static void text_message_callback(uint32_t from, uint32_t to, uint8_t channel, const char *text)
{
    received_count++;
    // Do your own thing here. This example just prints the message to the serial console.
    Serial.print("Received a text message on channel: ");
    Serial.print(channel);
    Serial.print(" from: ");
    Serial.print(from);
    Serial.print(" to: ");
    Serial.print(to);
    Serial.print(" message: ");
    Serial.println(text);
    if (to == BROADCAST_ADDR)
    {
        Serial.println("This is a BROADCAST message.");
    }
    else if (to == my_node_num)
    {
        Serial.println("This is a DM to me!");
    }
    else
    {
        Serial.println("This is a DM to someone else.");
    }
}
