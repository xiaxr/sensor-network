#pragma once

/*
 Copyright (C) 2011 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "RF24Network_config.h"

#include <cstddef>
#include <cstdint>

#include <assert.h>
#include <atomic>
#include <map>
#include <queue>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <utility> // std::pair
#include <vector>

namespace rf24_net {
uint16_t next_message_id();
}

/* Header types range */
#define MIN_USER_DEFINED_HEADER_TYPE 0
#define MAX_USER_DEFINED_HEADER_TYPE 127

/**
 * A NETWORK_ADDR_RESPONSE type is utilized to manually route custom messages
 * containing a single RF24Network address
 *
 * Used by RF24Mesh
 *
 * If a node receives a message of this type that is directly addressed to it,
 * it will read the included message, and forward the payload on to the proper
 * recipient. <br> This allows nodes to forward multicast messages to the master
 * node, receive a response, and forward it back to the requester.
 */
#define NETWORK_ADDR_RESPONSE 128

/**
 * Messages of type NETWORK_PING will be dropped automatically by the recipient.
 * A NETWORK_ACK or automatic radio-ack will indicate to the sender whether the
 * payload was successful. The time it takes to successfully send a NETWORK_PING
 * is the round-trip-time.
 */
#define NETWORK_PING 130

/**
 * External data types are used to define messages that will be passed to an
 * external data system. This allows RF24Network to route and pass any type of
 * data, such as TCP/IP frames, while still being able to utilize standard
 * RF24Network messages etc.
 *
 * **Linux**
 * Linux devices (defined RF24_LINUX) will buffer all data types in the user
 * cache.
 *
 * **Arduino/AVR/Etc:** Data transmitted with the type set to EXTERNAL_DATA_TYPE
 * will not be loaded into the user cache. <br> External systems can extract
 * external data using the following process, while internal data types are
 * cached in the user buffer, and accessed using network.read() :
 * @code
 * uint8_t return_type = network.update();
 * if(return_type == EXTERNAL_DATA_TYPE){
 *     uint16_t size = network.frag_ptr->message_size;
 *     memcpy(&myDataBuffer,network.frag_ptr->message_buffer,network.frag_ptr->message_size);
 * }
 * @endcode
 */
#define EXTERNAL_DATA_TYPE 131

/**
 * Messages of this type designate the first of two or more message fragments,
 * and will be re-assembled automatically.
 */
#define NETWORK_FIRST_FRAGMENT 148

/**
 * Messages of this type indicate a fragmented payload with two or more message
 * fragments.
 */
#define NETWORK_MORE_FRAGMENTS 149

/**
 * Messages of this type indicate the last fragment in a sequence of message
 * fragments. Messages of this type do not receive a NETWORK_ACK
 */
#define NETWORK_LAST_FRAGMENT 150
//#define NETWORK_LAST_FRAGMENT 201

// NO ACK Response Types
//#define NETWORK_ACK_REQUEST 192

/**
 * Messages of this type are used internally, to signal the sender that a
 * transmission has been completed. RF24Network does not directly have a
 * built-in transport layer protocol, so message delivery is not 100%
 * guaranteed.<br> Messages can be lost via corrupted dynamic payloads, or a
 * NETWORK_ACK can fail, while the message was actually successful.
 *
 * NETWORK_ACK messages can be utilized as a traffic/flow control mechanism,
 * since transmitting nodes will be forced to wait until the payload is
 * transmitted across the network and acknowledged, before sending additional
 * data.
 *
 * In the event that the transmitting device will be waiting for a direct
 * response, manually sent by the recipient, a NETWORK_ACK is not required. <br>
 * User messages utilizing a 'type' with a decimal value of 64 or less will not
 * be acknowledged across the network via NETWORK_ACK messages.
 */
#define NETWORK_ACK 193

/**
 * Used by RF24Mesh
 *
 * Messages of this type are used with multi-casting , to find active/available
 * nodes. Any node receiving a NETWORK_POLL sent to a multicast address will
 * respond directly to the sender with a blank message, indicating the address
 * of the available node via the header.
 */
#define NETWORK_POLL 194

/**
 * Used by RF24Mesh
 *
 * Messages of this type are used to request information from the master node,
 * generally via a unicast (direct) write. Any (non-master) node receiving a
 * message of this type will manually forward it to the master node using an
 * normal network write.
 */
#define NETWORK_REQ_ADDRESS 195
//#define NETWORK_ADDR_LOOKUP 196
//#define NETWORK_ADDR_RELEASE 197
/** @} */

#define NETWORK_MORE_FRAGMENTS_NACK 200

/** Internal defines for handling written payloads */
#define TX_NORMAL 0
#define TX_ROUTED 1
#define USER_TX_TO_PHYSICAL_ADDRESS 2 // no network ACK
#define USER_TX_TO_LOGICAL_ADDRESS 3 // network ACK
#define USER_TX_MULTICAST 4

#define MAX_FRAME_SIZE 32 // Size of individual radio frames
#define FRAME_HEADER_SIZE 10 // Size of RF24Network frames - data

#define USE_CURRENT_CHANNEL 255

/** Internal defines for handling internal payloads - prevents reading
 * additional data from the radio when buffers are full */
#define FLAG_HOLD_INCOMING 1
/** FLAG_BYPASS_HOLDS is mainly for use with RF24Mesh as follows:
 * a: Ensure no data in radio buffers, else exit
 * b: Address is changed to multicast address for renewal
 * c: Holds Cleared (bypass flag is set)
 * d: Address renewal takes place and is set
 * e: Holds Enabled (bypass flag off)
 */
#define FLAG_BYPASS_HOLDS 2
#define FLAG_FAST_FRAG 4
#define FLAG_NO_POLL 8

class RF24;

using address_t = uint16_t;

/**
 * Header which is sent with each message
 *
 * The frame put over the air consists of this header and a message
 *
 * Headers are addressed to the appropriate node, and the network forwards them
 * on to their final destination.
 */
struct RF24NetworkHeader {
  address_t from_node; /**< Logical address where the message was generated */
  address_t to_node;   /**< Logical address where the message is going */
  uint16_t id; /**< Sequential message ID, incremented every time a new frame is
                  constructed */
  /**
   * Message Types:
   * User message types 1 through 64 will NOT be acknowledged by the network,
   * while message types 65 through 127 will receive a network ACK. System
   * message types 192 through 255 will NOT be acknowledged by the network.
   * Message types 128 through 192 will receive a network ACK. <br> <br><br>
   */
  unsigned char type; /**< <b>Type of the packet. </b> 0-127 are user-defined
                         types, 128-255 are reserved for system */

  /**
   * During fragmentation, it carries the fragment_id, and on the last fragment
   * it carries the header_type.<br>
   */
  unsigned char reserved; /**< *Reserved for system use* */

  /**
   * Default constructor
   */

  RF24NetworkHeader() {}

  /**
   * Send constructor
   *
   * @note Now supports automatic fragmentation for very long messages, which
   * can be sent as usual if fragmentation is enabled.
   *
   * Fragmentation is enabled by default for all devices except ATTiny <br>
   * Configure fragmentation and max payload size in RF24Network_config.h
   *
   * Use this constructor to create a header and then send a message
   *
   * @code
   *  uint16_t recipient_address = 011;
   *
   *  RF24NetworkHeader header(recipient_address,'t');
   *
   *  network.write(header,&message,sizeof(message));
   * @endcode
   *
   * @param _to The Octal format, logical node address where the message is
   * going
   * @param _type The type of message which follows.  Only 0-127 are allowed for
   * user messages. Types 1-64 will not receive a network acknowledgement.
   */

  RF24NetworkHeader(address_t _to, unsigned char _type = 0)
      : to_node(_to), id(rf24_net::next_message_id()), type(_type) {}
} __attribute__((packed));

/**
 * Frame structure for internal message handling, and for use by external
 * applications
 *
 * The actual frame put over the air consists of a header (8-bytes) and a
 * message payload (Up to 24-bytes)<br> When data is received, it is stored
 * using the RF24NetworkFrame structure, which includes:
 * 1. The header
 * 2. The size of the included message
 * 3. The 'message' or data being received
 *
 *
 */

struct RF24NetworkFrame {
  RF24NetworkHeader header; /**< Header which is sent with each message */
  uint16_t message_size;    /**< The size in bytes of the payload length */
  uint8_t message_buffer[MAX_PAYLOAD_SIZE]; //< Array to store the message

  RF24NetworkFrame() {}
  RF24NetworkFrame(RF24NetworkHeader &_header, const void *_message = NULL,
                   uint16_t _len = 0)
      : header(_header), message_size(_len) {
    if (_message && _len) {
      memcpy(message_buffer, _message, _len);
    }
  }
} __attribute__((packed));

/**
 * 2014-2020 - Optimized Network Layer for RF24 Radios
 *
 * This class implements an OSI Network Layer using nRF24L01(+) radios driven
 * by RF24 library.
 */

class RF24Network {

  /**
   * @name Primary Interface
   *
   *  These are the main methods you need to operate the network
   */
  /**@{*/

public:
  /**
   * Construct the network
   *
   * @param _radio The underlying radio driver instance
   *
   */

  RF24Network(RF24 &_radio);

  /**
   * Bring up the network using the current radio frequency/channel.
   * Calling begin brings up the network, and configures the address, which
   * designates the location of the node within RF24Network topology.
   * @note Node addresses are specified in Octal format, see [RF24Network
   * Addressing](Addressing.html) for more information.
   * @warning Be sure to 'begin' the radio first.
   *
   * **Example 1:** Begin on current radio channel with address 0 (master node)
   * @code
   * network.begin(00);
   * @endcode
   * **Example 2:** Begin with address 01 (child of master)
   * @code
   * network.begin(01);
   * @endcode
   * **Example 3:** Begin with address 011 (child of 01, grandchild of master)
   * @code
   * network.begin(011);
   * @endcode
   *
   * @see begin(uint8_t _channel, uint16_t _node_address )
   * @param _node_address The logical address of this node
   *
   */
  inline void begin(uint16_t _node_address) {
    begin(USE_CURRENT_CHANNEL, _node_address);
  }

  /**
   * Main layer loop
   *
   * This function must be called regularly to keep the layer going.  This is
   * where payloads are re-routed, received, and all the action happens.
   *
   * @return Returns the type of the last received payload.
   */
  uint8_t update();

  /**
   * Test whether there is a message available for this node
   *
   * @return Whether there is a message available for this node
   */
  bool available() { return (!frame_queue.empty()); }

  /**
   * Read the next available header
   *
   * Reads the next available header without advancing to the next
   * incoming message.  Useful for doing a switch on the message type
   *
   * If there is no message available, the header is not touched
   *
   * @param[out] header The header (envelope) of the next message
   */
  uint16_t peek(RF24NetworkHeader &header);

  /**
   * Read the next available payload
   *
   * Reads the next available payload without advancing to the next
   * incoming message.  Useful for doing a transparent packet
   * manipulation layer on top of RF24Network.
   *
   * @param[out] header The header (envelope) of this message
   * @param[out] message Pointer to memory where the message should be placed
   * @param maxlen Amount of bytes to copy to message.
   */
  void peek(RF24NetworkHeader &header, void *message, uint16_t maxlen);

  /**
   * Send a message
   *
   * @note RF24Network now supports fragmentation for very long messages, send
   * as normal. Fragmentation may need to be enabled or configured by editing
   * the RF24Network_config.h file. Default max payload size is 120 bytes.
   *
   * @code
   * uint32_t time = millis();
   * uint16_t to = 00; // Send to master
   * RF24NetworkHeader header(to, 'T'); // Send header type 'T'
   * network.write(header,&time,sizeof(time));
   * @endcode
   * @param[in,out] header The header (envelope) of this message.  The critical
   * thing to fill in is the @p to_node field so we know where to send the
   * message.  It is then updated with the details of the actual header sent.
   * @param message Pointer to memory where the message is located
   * @param len The size of the message
   * @return Whether the message was successfully received
   */
  bool write(RF24NetworkHeader &header, const void *message, uint16_t len);

  /**@}*/
  /**
   * @name Advanced Configuration
   *
   *  For advanced configuration of the network
   */
  /**@{*/

  /**
   * By default, multicast addresses are divided into levels.
   *
   * Nodes 1-5 share a multicast address, nodes n1-n5 share a multicast address,
   * and nodes n11-n55 share a multicast address.<br>
   *
   * This option is used to override the defaults, and create custom multicast
   * groups that all share a single address. <br> The level should be specified
   * in decimal format 1-6 <br>
   * @see multicastRelay
   * @param level Levels 1 to 6 are available. All nodes at the same level will
   * receive the same messages if in range. Messages will be routed in order of
   * level, low to high by default, with the master node (00) at multicast Level
   * 0
   */

  void multicastLevel(uint8_t level);

  /**
   * Enabling this will allow this node to automatically forward received
   * multicast frames to the next highest multicast level. Duplicate frames are
   * filtered out, so multiple forwarding nodes at the same level should not
   * interfere. Forwarded payloads will also be received.
   * @see multicastLevel
   */

  bool multicastRelay;

  /**
   * @note: This value is automatically assigned based on the node address
   * to reduce errors and increase throughput of the network.
   *
   * Sets the timeout period for individual payloads in milliseconds at
   * staggered intervals. Payloads will be retried automatically until success
   * or timeout Set to 0 to use the normal auto retry period defined by
   * radio.setRetries()
   *
   */

  uint32_t txTimeout; /**< Network timeout value */

  /**
   * This only affects payloads that are routed by one or more nodes.
   * This specifies how long to wait for an ack from across the network.
   * Radios sending directly to their parent or children nodes do not
   * utilize this value.
   */

  uint16_t routeTimeout; /**< Timeout for routed payloads */

  /**@}*/
  /**
   * @name Advanced Operation
   *
   *  For advanced operation of the network
   */
  /**@{*/

  /**
   * Return the number of failures and successes for all transmitted payloads,
   * routed or sent directly
   * @note This needs to be enabled via `#define ENABLE_NETWORK_STATS` in
   * RF24Network_config.h
   *
   *   @code
   * bool fails, success;
   * network.failures(&fails,&success);
   * @endcode
   *
   */
  void failures(uint32_t *_fails, uint32_t *_ok);

#if defined(RF24NetworkMulticast)

  /**
   * Send a multicast message to multiple nodes at once
   * Allows messages to be rapidly broadcast through the network
   *
   * Multicasting is arranged in levels, with all nodes on the same level
   * listening to the same address Levels are assigned by network level ie:
   * nodes 01-05: Level 1, nodes 011-055: Level 2
   * @see multicastLevel
   * @see multicastRelay
   * @param header reference to the RF24NetworkHeader object used for this @p
   * message
   * @param message Pointer to memory where the message is located
   * @param len The size of the message
   * @param level Multicast level to broadcast to
   * @return Whether the message was successfully sent
   */

  bool multicast(RF24NetworkHeader &header, const void *message, uint16_t len,
                 uint8_t level);

#endif

  /**
   * Writes a direct (unicast) payload. This allows routing or sending messages
   * outside of the usual routing paths. The same as write, but a physical
   * address is specified as the last option. The payload will be written to the
   * physical address, and routed as necessary by the recipient
   */
  bool write(RF24NetworkHeader &header, const void *message, uint16_t len,
             uint16_t writeDirect);

  /**
   * This node's parent address
   *
   * @return This node's parent address, or -1 if this is the base
   */
  uint16_t parent() const {
    if (node_address == 0)
      return -1;
    else
      return parent_node;
  }
  /**
   * Provided a node address and a pipe number, will return the RF24Network
   * address of that child pipe for that node
   */
  uint16_t addressOfPipe(uint16_t node, uint8_t pipeNo);

  /**
   * @note Addresses are specified in octal: 011, 034
   * @return True if a supplied address is valid
   */
  bool is_valid_address(uint16_t node);

  /**@}*/
  /**
   * @name Deprecated
   *
   *  Maintained for backwards compatibility
   */
  /**@{*/

  /**
   * Bring up the network on a specific radio frequency/channel.
   * @note Use radio.setChannel() to configure the radio channel
   *
   * **Example 1:** Begin on channel 90 with address 0 (master node)
   * @code
   * network.begin(90,0);
   * @endcode
   * **Example 2:** Begin on channel 90 with address 01 (child of master)
   * @code
   * network.begin(90,01);
   * @endcode
   * **Example 3:** Begin on channel 90 with address 011 (child of 01,
   * grandchild of master)
   * @code
   * network.begin(90,011);
   * @endcode
   *
   * @param _channel The RF channel to operate on
   * @param _node_address The logical address of this node
   *
   */
  void begin(uint8_t _channel, uint16_t _node_address);

  /**@}*/
  /**
   * @name External Applications/Systems
   *
   *  Interface for External Applications and Systems ( RF24Mesh, RF24Ethernet )
   */
  /**@{*/

  /** The raw system frame buffer of received data. */

  uint8_t frame_buffer[MAX_FRAME_SIZE];

  /**
   * **Linux** <br>
   * Data with a header type of EXTERNAL_DATA_TYPE will be loaded into a
   * separate queue. The data can be accessed as follows:
   * @code
   * RF24NetworkFrame f;
   * while(network.external_queue.size() > 0){
   *   f = network.external_queue.front();
   *   uint16_t dataSize = f.message_size;
   *   //read the frame message buffer
   *   memcpy(&myBuffer,&f.message_buffer,dataSize);
   *   network.external_queue.pop();
   * }
   * @endcode
   */
  std::queue<RF24NetworkFrame> external_queue;
  std::queue<RF24NetworkFrame> frame_queue;

  /**
   * Variable to determine whether update() will return after the radio buffers
   * have been emptied (DEFAULT), or whether to return immediately when (most)
   * system types are received.
   *
   * As an example, this is used with RF24Mesh to catch and handle system
   * messages without loading them into the user cache.
   *
   * The following reserved/system message types are handled automatically, and
   * not returned.
   *
   * | System Message Types <br> (Not Returned) |
   * |-----------------------|
   * | NETWORK_ADDR_RESPONSE |
   * | NETWORK_ACK           |
   * | NETWORK_PING          |
   * | NETWORK_POLL <br>(With multicast enabled) |
   * | NETWORK_REQ_ADDRESS   |
   *
   */
  bool returnSysMsgs;

  /**
   * Network Flags allow control of data flow
   *
   * Incoming Blocking: If the network user-cache is full, lets radio cache fill
   * up. Radio ACKs are not sent when radio internal cache is full.<br> This
   * behaviour may seem to result in more failed sends, but the payloads would
   * have otherwise been dropped due to the cache being full.<br>
   *
   * | FLAGS | Value | Description |
   * |-------|-------|-------------|
   * |FLAG_HOLD_INCOMING| 1(bit_1) | INTERNAL: Set automatically when a
   * fragmented payload will exceed the available cache | |FLAG_BYPASS_HOLDS|
   * 2(bit_2) | EXTERNAL: Can be used to prevent holds from blocking. Note:
   * Holds are disabled & re-enabled by RF24Mesh when renewing addresses. This
   * will cause data loss if incoming data exceeds the available cache space|
   * |FLAG_FAST_FRAG| 4(bit_3) | INTERNAL: Replaces the fastFragTransfer
   * variable, and allows for faster transfers between directly connected nodes.
   * | |FLAG_NO_POLL| 8(bit_4) | EXTERNAL/USER: Disables NETWORK_POLL responses
   * on a node-by-node basis. |
   *
   */
  uint8_t networkFlags;

private:
  bool write(uint16_t, uint8_t directTo);
  bool write_to_pipe(uint16_t node, uint8_t pipe, bool multicast);
  uint8_t enqueue(RF24NetworkHeader *header);

  bool is_direct_child(uint16_t node);
  bool is_descendant(uint16_t node);

  uint16_t direct_child_route_to(uint16_t node);
  void setup_address(void);
  bool _write(RF24NetworkHeader &header, const void *message, uint16_t len,
              uint16_t writeDirect);

  struct logicalToPhysicalStruct {
    uint16_t send_node;
    uint8_t send_pipe;
    bool multicast;
  };

  void logicalToPhysicalAddress(logicalToPhysicalStruct *conversionInfo);

  RF24 &radio; /**< Underlying radio driver, provides link/physical layers */

#if defined(RF24NetworkMulticast)
  uint8_t multicast_level;
#endif

  uint16_t
      node_address; /**< Logical node address of this unit, 1 .. UINT_MAX */
  uint8_t frame_size;
  const static unsigned int max_frame_payload_size =
      MAX_FRAME_SIZE - sizeof(RF24NetworkHeader);

public:
  std::map<uint16_t, RF24NetworkFrame> frameFragmentsCache;

private:
  bool appendFragmentToFrame(RF24NetworkFrame frame);

  uint16_t parent_node; /**< Our parent's node address */
  uint8_t parent_pipe;  /**< The pipe our parent uses to listen to us */
  uint16_t node_mask;   /**< The bits which contain signfificant node address
                           information */
  uint64_t pipe_address(uint16_t node, uint8_t pipe);

#if defined ENABLE_NETWORK_STATS
  uint32_t nFails;
  uint32_t nOK;
#endif

#if defined(RF24NetworkMulticast)
  uint16_t levelToAddress(uint8_t level);
#endif

  /** @} */
public:
};
