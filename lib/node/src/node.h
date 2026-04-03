// Node connection
// Handle Meshtastic node (XIAO-nRF52840) connection via uart.

#ifndef NODE_H
#define NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize node connection
 */
void node_init(void);

/**
 * Is node connected
 * 
 * @return  true if node is connected
 */
bool node_is_connected(void);

/**
 * How many messages have been received
 * 
 * @return  Number of messages received
 */
uint32_t node_get_received_count(void);

/**
 * How many messages have been sent
 * 
 * @return  Number of messages sent
 */
uint32_t node_get_sent_count(void);

/**
 * Handle node loop
 */
void node_loop(void);

/**
 * Send message to node for transmission
 * 
 * @param channel_index index of channel where message will be sent
 * @param msg           pointer to message
 * @return              true if sending was succesful
 */
bool node_send_message(uint8_t channel_index, const char *msg);

#ifdef __cplusplus
}
#endif

#endif // NODE_H
