/*
 Copyright (C) 2011 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
#include "RF24Network_config.h"

#include "RF24Network.h"
#include <RF24X/RF24.h>

#include <algorithm>
#include <atomic>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

namespace {
auto next_message_id_ = std::atomic_uint16_t(1);
}

namespace rf24_net {
uint16_t next_message_id() { return next_message_id_++; }
} // namespace rf24_net

/******************************************************************/
RF24Network::RF24Network(RF24 &_radio)
    : radio(_radio), frame_size(MAX_FRAME_SIZE) {
  networkFlags = 0;
  returnSysMsgs = 0;
  multicastRelay = 0;
}

void RF24Network::begin(uint8_t _channel, uint16_t _node_address) {
  if (!is_valid_address(_node_address))
    return;

  node_address = _node_address;

  if (!radio.isValid()) {
    return;
  }

  // Set up the radio the way we want it to look
  if (_channel != USE_CURRENT_CHANNEL) {
    radio.setChannel(_channel);
  }
  // radio.enableDynamicAck();
  radio.setAutoAck(1);
  radio.setAutoAck(0, 0);

#if defined(ENABLE_DYNAMIC_PAYLOADS)
  radio.enableDynamicPayloads();
#endif

  // Use different retry periods to reduce data collisions
  uint8_t retryVar = (((node_address % 6) + 1) * 2) + 3;
  radio.setRetries(retryVar, 5); // max about 85ms per attempt
  txTimeout = 25;
  routeTimeout =
      txTimeout * 3; // Adjust for max delay per node within a single chain

  // Setup our address helper cache
  setup_address();

  // Open up all listening pipes
  uint8_t i = 6;
  while (i--) {
    radio.openReadingPipe(i, pipe_address(_node_address, i));
  }
  radio.startListening();
}

/******************************************************************/

#if defined ENABLE_NETWORK_STATS
void RF24Network::failures(uint32_t *_fails, uint32_t *_ok) {
  *_fails = nFails;
  *_ok = nOK;
}
#endif

/******************************************************************/

uint8_t RF24Network::update(void) {

  uint8_t returnVal = 0;

  uint32_t timeout = millis();

  while (radio.available()) {
    if (millis() - timeout > 1000) {
#if defined FAILURE_HANDLING
      radio.failureDetected = 1;
#endif
      break;
    }
#if defined(ENABLE_DYNAMIC_PAYLOADS) && !defined(XMEGA_D3)
    frame_size = radio.getDynamicPayloadSize();
#else
    frame_size = MAX_FRAME_SIZE;
#endif

    // Fetch the payload, and see if this was the last one.
    radio.read(frame_buffer, frame_size);

    // Read the beginning of the frame as the header
    RF24NetworkHeader *header = (RF24NetworkHeader *)(&frame_buffer);

    // Throw it away if it's not a valid address or too small
    if (frame_size < sizeof(RF24NetworkHeader) ||
        !is_valid_address(header->to_node) ||
        !is_valid_address(header->from_node)) {
      continue;
    }

    returnVal = header->type;
    // Is this for us?
    if (header->to_node == node_address) {

      if (header->type == NETWORK_PING) {
        continue;
      }
      if (header->type == NETWORK_ADDR_RESPONSE) {
        uint16_t requester = NETWORK_DEFAULT_ADDRESS;
        if (requester != node_address) {
          header->to_node = requester;
          write(header->to_node, USER_TX_TO_PHYSICAL_ADDRESS);
          continue;
        }
      }
      if (header->type == NETWORK_REQ_ADDRESS && node_address) {
        header->from_node = node_address;
        header->to_node = 0;
        write(header->to_node, TX_NORMAL);
        continue;
      }
      if ((returnSysMsgs && header->type > 127) ||
          header->type == NETWORK_ACK) {
        if (header->type != NETWORK_FIRST_FRAGMENT &&
            header->type != NETWORK_MORE_FRAGMENTS &&
            header->type != EXTERNAL_DATA_TYPE &&
            header->type != NETWORK_LAST_FRAGMENT) {
          return returnVal;
        }
      }

      if (enqueue(header) == 2) { // External data received
        return EXTERNAL_DATA_TYPE;
      }

    } else {
#if defined(RF24NetworkMulticast)

      if (header->to_node == 0100) {
        if (header->type == NETWORK_POLL) {
          returnVal = 0;
          if (!(networkFlags & FLAG_NO_POLL) &&
              node_address != NETWORK_DEFAULT_ADDRESS) {
            header->to_node = header->from_node;
            header->from_node = node_address;
            delay(parent_pipe);
            write(header->to_node, USER_TX_TO_PHYSICAL_ADDRESS);
          }
          continue;
        }

        uint8_t val = enqueue(header);

        if (multicastRelay) {
          if ((node_address >> 3) != 0) {
            // for all but the first level of nodes, those not directly
            // connected to the master, we add the total delay per level
            delayMicroseconds(600 * 4);
          }
          delayMicroseconds((node_address % 4) * 600);
          write(levelToAddress(multicast_level) << 3, 4);
        }
        if (val == 2) { // External data received
          return EXTERNAL_DATA_TYPE;
        }

      } else {
        if (node_address != NETWORK_DEFAULT_ADDRESS) {
          write(header->to_node,
                1); // Send it on, indicate it is a routed payload
          returnVal = 0;
        }
      }
#else
      if (node_address != NETWORK_DEFAULT_ADDRESS) {
        write(header->to_node, 1); // Send it on, indicate it is a routed
                                   // payload
        returnVal = 0;
      }
#endif
    }

  } // radio.available()
  return returnVal;
}

uint8_t RF24Network::enqueue(RF24NetworkHeader *header) {
  uint8_t result = false;

  RF24NetworkFrame frame =
      RF24NetworkFrame(*header, frame_buffer + sizeof(RF24NetworkHeader),
                       frame_size - sizeof(RF24NetworkHeader));

  bool isFragment = (frame.header.type == NETWORK_FIRST_FRAGMENT ||
                     frame.header.type == NETWORK_MORE_FRAGMENTS ||
                     frame.header.type == NETWORK_LAST_FRAGMENT);

  // This is sent to itself
  if (frame.header.from_node == node_address) {
    if (isFragment) {
      result = false;
    } else if (frame.header.id > 0) {
      frame_queue.push(frame);
      result = true;
    }
  } else if (isFragment) {
    // The received frame contains the a fragmented payload
    // Set the more fragments flag to indicate a fragmented frame
    // Append payload
    result = appendFragmentToFrame(frame);

    // The header.reserved contains the actual header.type on the last fragment
    if (result && frame.header.type == NETWORK_LAST_FRAGMENT) {

      RF24NetworkFrame *f = &(frameFragmentsCache[frame.header.from_node]);

      result = f->header.type == EXTERNAL_DATA_TYPE ? 2 : 1;

      if (f->header.id > 0 && f->message_size > 0) {
        // Load external payloads into a separate queue on linux
        if (result == 2) {
          external_queue.push(frameFragmentsCache[frame.header.from_node]);
        } else {
          frame_queue.push(frameFragmentsCache[frame.header.from_node]);
        }
      }
      frameFragmentsCache.erase(frame.header.from_node);
    }

  } else { //  if (frame.header.type <= MAX_USER_DEFINED_HEADER_TYPE) {
           // This is not a fragmented payload but a whole frame.
    // Copy the current frame into the frame queue
    result = frame.header.type == EXTERNAL_DATA_TYPE ? 2 : 1;
    // Load external payloads into a separate queue on linux
    if (result == 2) {
      external_queue.push(frame);
    } else {
      frame_queue.push(frame);
    }

  } /* else {
     //Undefined/Unknown header.type received. Drop frame!
     //The frame is not explicitly dropped, but the given object is ignored.
     //FIXME: does this causes problems with memory management?
   }*/

  return result;
}

/******************************************************************/

bool RF24Network::appendFragmentToFrame(RF24NetworkFrame frame) {

  // This is the first of 2 or more fragments.
  if (frame.header.type == NETWORK_FIRST_FRAGMENT) {
    if (frameFragmentsCache.count(frame.header.from_node) != 0) {
      RF24NetworkFrame *f = &(frameFragmentsCache[frame.header.from_node]);
      // Already rcvd first frag
      if (f->header.id == frame.header.id) {
        return false;
      }
    }

    frameFragmentsCache[frame.header.from_node] = frame;
    return true;
  } else

      if (frame.header.type == NETWORK_MORE_FRAGMENTS) {

    if (frameFragmentsCache.count(frame.header.from_node) < 1) {
      return false;
    }
    RF24NetworkFrame *f = &(frameFragmentsCache[frame.header.from_node]);

    if (f->message_size + frame.message_size > MAX_PAYLOAD_SIZE) {
      return false;
    }

    if (f->header.reserved - 1 == frame.header.reserved &&
        f->header.id == frame.header.id) {
      // Cache the fragment
      memcpy(f->message_buffer + f->message_size, frame.message_buffer,
             frame.message_size);
      f->message_size += frame.message_size; // Increment message size
      f->header = frame.header;              // Update header
      return true;

    } else {
      return false;
    }

  } else if (frame.header.type == NETWORK_LAST_FRAGMENT) {

    // We have received the last fragment
    if (frameFragmentsCache.count(frame.header.from_node) < 1) {
      return false;
    }
    // Create pointer to the cached frame
    RF24NetworkFrame *f = &(frameFragmentsCache[frame.header.from_node]);

    if (f->message_size + frame.message_size > MAX_PAYLOAD_SIZE) {
      return false;
    }
    // Error checking for missed fragments and payload size
    if (f->header.reserved - 1 != 1 || f->header.id != frame.header.id) {
      // frameFragmentsCache.erase(
      // std::make_pair(frame.header.id,frame.header.from_node) );
      return false;
    }
    // The user specified header.type is sent with the last fragment in the
    // reserved field
    frame.header.type = frame.header.reserved;
    frame.header.reserved = 1;

    // Append the received fragment to the cached frame
    memcpy(f->message_buffer + f->message_size, frame.message_buffer,
           frame.message_size);
    f->message_size += frame.message_size; // Increment message size
    f->header = frame.header;              // Update header
    return true;
  }
  return false;
}

uint16_t RF24Network::peek(RF24NetworkHeader &header) {
  if (available()) {
    RF24NetworkFrame frame = frame_queue.front();
    memcpy(&header, &frame.header, sizeof(RF24NetworkHeader));
    return frame.message_size;
  }
  return 0;
}

/******************************************************************/

void RF24Network::peek(RF24NetworkHeader &header, void *message,
                       uint16_t maxlen) {
  if (available()) { // TODO: Untested
    RF24NetworkFrame frame = frame_queue.front();
    memcpy(&header, &(frame.header), sizeof(RF24NetworkHeader));
    memcpy(message, frame.message_buffer, maxlen);
  }
}

#if defined RF24NetworkMulticast
/******************************************************************/
bool RF24Network::multicast(RF24NetworkHeader &header, const void *message,
                            uint16_t len, uint8_t level) {
  // Fill out the header
  header.to_node = 0100;
  header.from_node = node_address;
  return write(header, message, len, levelToAddress(level));
}
#endif

/******************************************************************/
bool RF24Network::write(RF24NetworkHeader &header, const void *message,
                        uint16_t len) {
  return write(header, message, len, 070);
}

/******************************************************************/
bool RF24Network::write(RF24NetworkHeader &header, const void *message,
                        uint16_t len, uint16_t writeDirect) {

#if defined(DISABLE_FRAGMENTATION)
  frame_size = rf24_min(len + sizeof(RF24NetworkHeader), MAX_FRAME_SIZE);
  return _write(header, message, rf24_min(len, max_frame_payload_size),
                writeDirect);
#else
  if (len <= max_frame_payload_size) {
    // Normal Write (Un-Fragmented)
    frame_size = len + sizeof(RF24NetworkHeader);
    if (_write(header, message, len, writeDirect)) {
      return 1;
    }
    return 0;
  }
  // Check payload size
  if (len > MAX_PAYLOAD_SIZE) {
    return false;
  }

  // Divide the message payload into chunks of max_frame_payload_size
  uint8_t fragment_id =
      (len % max_frame_payload_size != 0) +
      ((len) / max_frame_payload_size); // the number of fragments to send =
                                        // ceil(len/max_frame_payload_size)

  uint8_t msgCount = 0;

  if (header.to_node != 0100) {
    networkFlags |= FLAG_FAST_FRAG;
    radio.stopListening();
  }

  uint8_t retriesPerFrag = 0;
  uint8_t type = header.type;
  bool ok = 0;

  while (fragment_id > 0) {

    // Copy and fill out the header
    // RF24NetworkHeader fragmentHeader = header;
    header.reserved = fragment_id;

    if (fragment_id == 1) {
      header.type = NETWORK_LAST_FRAGMENT; // Set the last fragment flag to
                                           // indicate the last fragment
      header.reserved =
          type; // The reserved field is used to transmit the header type
    } else {
      if (msgCount == 0) {
        header.type = NETWORK_FIRST_FRAGMENT;
      } else {
        header.type = NETWORK_MORE_FRAGMENTS; // Set the more fragments flag to
                                              // indicate a fragmented frame
      }
    }

    uint16_t offset = msgCount * max_frame_payload_size;
    uint16_t fragmentLen =
        rf24_min((uint16_t)(len - offset), max_frame_payload_size);

    // Try to send the payload chunk with the copied header
    frame_size = sizeof(RF24NetworkHeader) + fragmentLen;
    ok = _write(header, ((char *)message) + offset, fragmentLen, writeDirect);

    if (!ok) {
      delay(2);
      ++retriesPerFrag;
    } else {
      retriesPerFrag = 0;
      fragment_id--;
      msgCount++;
    }

    // if(writeDirect != 070){ delay(2); } //Delay 2ms between sending multicast
    // payloads

    if (!ok && retriesPerFrag >= 3) {
      break;
    }

    // Message was successful sent
  }
  header.type = type;
  if (networkFlags & FLAG_FAST_FRAG) {
    ok = radio.txStandBy(txTimeout);
    radio.startListening();
    radio.setAutoAck(0, 0);
  }
  networkFlags &= ~FLAG_FAST_FRAG;

  // Return true if all the chunks where sent successfully

  if (!ok || fragment_id > 0) {
    return false;
  }
  return true;

#endif // Fragmentation enabled
}
/******************************************************************/

bool RF24Network::_write(RF24NetworkHeader &header, const void *message,
                         uint16_t len, uint16_t writeDirect) {
  // Fill out the header
  header.from_node = node_address;

  // Build the full frame to send
  memcpy(frame_buffer, &header, sizeof(RF24NetworkHeader));

  if (len) {
    memcpy(frame_buffer + sizeof(RF24NetworkHeader), message,
           rf24_min(frame_size - sizeof(RF24NetworkHeader), len));
  }

  // If the user is trying to send it to himself
  /*if ( header.to_node == node_address ){
        #if defined (RF24_LINUX)
          RF24NetworkFrame frame =
  RF24NetworkFrame(header,message,rf24_min(MAX_FRAME_SIZE-sizeof(RF24NetworkHeader),len));
        #else
      RF24NetworkFrame frame(header,len);
    #endif
        // Just queue it in the received queue
    return enqueue(frame);
  }*/
  // Otherwise send it out over the air

  if (writeDirect != 070) {
    uint8_t sendType =
        USER_TX_TO_LOGICAL_ADDRESS; // Payload is multicast to the first node,
                                    // and routed normally to the next

    if (header.to_node == 0100) {
      sendType = USER_TX_MULTICAST;
    }
    if (header.to_node == writeDirect) {
      sendType =
          USER_TX_TO_PHYSICAL_ADDRESS; // Payload is multicast to the first
                                       // node, which is the recipient
    }
    return write(writeDirect, sendType);
  }
  return write(header.to_node, TX_NORMAL);
}

/******************************************************************/

bool RF24Network::write(
    uint16_t to_node,
    uint8_t
        directTo) // Direct To: 0 = First Payload, standard routing, 1=routed
                  // payload, 2=directRoute to host, 3=directRoute to Route
{
  bool ok = false;
  bool isAckType = false;
  if (frame_buffer[6] > 64 && frame_buffer[6] < 192) {
    isAckType = true;
  }

  /*if( ( (frame_buffer[7] % 2) && frame_buffer[6] == NETWORK_MORE_FRAGMENTS) ){
        isAckType = 0;
  }*/

  // Throw it away if it's not a valid address
  if (!is_valid_address(to_node))
    return false;

  // Load info into our conversion structure, and get the converted address info
  logicalToPhysicalStruct conversion = {to_node, directTo, 0};
  logicalToPhysicalAddress(&conversion);

  /**Write it*/
  if (directTo == TX_ROUTED && conversion.send_node == to_node && isAckType) {
    delay(2);
  }
  ok = write_to_pipe(conversion.send_node, conversion.send_pipe,
                     conversion.multicast);

  if (directTo == TX_ROUTED && ok && conversion.send_node == to_node &&
      isAckType) {

    RF24NetworkHeader *header = (RF24NetworkHeader *)&frame_buffer;
    header->type = NETWORK_ACK; // Set the payload type to NETWORK_ACK
    header->to_node =
        header->from_node; // Change the 'to' address to the 'from' address

    conversion.send_node = header->from_node;
    conversion.send_pipe = TX_ROUTED;
    conversion.multicast = 0;
    logicalToPhysicalAddress(&conversion);

    // Write the data using the resulting physical address
    frame_size = sizeof(RF24NetworkHeader);
    write_to_pipe(conversion.send_node, conversion.send_pipe,
                  conversion.multicast);

    // dynLen=0;
  }

  if (ok && conversion.send_node != to_node &&
      (directTo == 0 || directTo == 3) && isAckType) {
    // Now, continue listening
    if (networkFlags & FLAG_FAST_FRAG) {
      radio.txStandBy(txTimeout);
      networkFlags &= ~FLAG_FAST_FRAG;
      radio.setAutoAck(0, 0);
    }
    radio.startListening();
    uint32_t reply_time = millis();

    while (update() != NETWORK_ACK) {
      delayMicroseconds(900);
      if (millis() - reply_time > routeTimeout) {
        ok = false;
        break;
      }
    }
  }
  if (!(networkFlags & FLAG_FAST_FRAG)) {
    // Now, continue listening
    radio.startListening();
  }

#if defined ENABLE_NETWORK_STATS
  if (ok == true) {
    ++nOK;
  } else {
    ++nFails;
  }
#endif
  return ok;
}

/******************************************************************/

// Provided the to_node and directTo option, it will return the resulting node
// and pipe
void RF24Network::logicalToPhysicalAddress(
    logicalToPhysicalStruct *conversionInfo) {

  // Create pointers so this makes sense.. kind of
  // We take in the to_node(logical) now, at the end of the function, output the
  // send_node(physical) address, etc. back to the original memory address that
  // held the logical information.
  uint16_t *to_node = &conversionInfo->send_node;
  uint8_t *directTo = &conversionInfo->send_pipe;
  bool *multicast = &conversionInfo->multicast;

  // Where do we send this?  By default, to our parent
  uint16_t pre_conversion_send_node = parent_node;

  // On which pipe
  uint8_t pre_conversion_send_pipe = parent_pipe;

  if (*directTo > TX_ROUTED) {
    pre_conversion_send_node = *to_node;
    *multicast = 1;
    // if(*directTo == USER_TX_MULTICAST || *directTo ==
    // USER_TX_TO_PHYSICAL_ADDRESS){
    pre_conversion_send_pipe = 0;
    //}
  }
  // If the node is a direct child,
  else if (is_direct_child(*to_node)) {
    // Send directly
    pre_conversion_send_node = *to_node;
    // To its listening pipe
    pre_conversion_send_pipe = 5;
  }
  // If the node is a child of a child
  // talk on our child's listening pipe,
  // and let the direct child relay it.
  else if (is_descendant(*to_node)) {
    pre_conversion_send_node = direct_child_route_to(*to_node);
    pre_conversion_send_pipe = 5;
  }

  *to_node = pre_conversion_send_node;
  *directTo = pre_conversion_send_pipe;
}

/********************************************************/

bool RF24Network::write_to_pipe(uint16_t node, uint8_t pipe, bool multicast) {
  bool ok = false;

  // Open the correct pipe for writing.
  // First, stop listening so we can talk

  if (!(networkFlags & FLAG_FAST_FRAG)) {
    radio.stopListening();
  }

  if (multicast) {
    radio.setAutoAck(0, 0);
  } else {
    radio.setAutoAck(0, 1);
  }

  radio.openWritingPipe(pipe_address(node, pipe));

  ok = radio.writeFast(frame_buffer, frame_size, 0);

  if (!(networkFlags & FLAG_FAST_FRAG)) {
    ok = radio.txStandBy(txTimeout);
    radio.setAutoAck(0, 0);
  }

  return ok;
}

/******************************************************************/

bool RF24Network::is_direct_child(uint16_t node) {
  bool result = false;

  // A direct child of ours has the same low numbers as us, and only
  // one higher number.
  //
  // e.g. node 0234 is a direct child of 034, and node 01234 is a
  // descendant but not a direct child

  // First, is it even a descendant?
  if (is_descendant(node)) {
    // Does it only have ONE more level than us?
    uint16_t child_node_mask = (~node_mask) << 3;
    result = (node & child_node_mask) == 0;
  }
  return result;
}

/******************************************************************/

bool RF24Network::is_descendant(uint16_t node) {
  return (node & node_mask) == node_address;
}

/******************************************************************/

void RF24Network::setup_address(void) {
  // First, establish the node_mask
  uint16_t node_mask_check = 0xFFFF;
#if defined(RF24NetworkMulticast)
  uint8_t count = 0;
#endif

  while (node_address & node_mask_check) {
    node_mask_check <<= 3;
#if defined(RF24NetworkMulticast)
    count++;
  }
  multicast_level = count;
#else
  }
#endif

  node_mask = ~node_mask_check;

  // parent mask is the next level down
  uint16_t parent_mask = node_mask >> 3;

  // parent node is the part IN the mask
  parent_node = node_address & parent_mask;

  // parent pipe is the part OUT of the mask
  uint16_t i = node_address;
  uint16_t m = parent_mask;
  while (m) {
    i >>= 3;
    m >>= 3;
  }
  parent_pipe = i;
}

/******************************************************************/
uint16_t RF24Network::addressOfPipe(uint16_t node, uint8_t pipeNo) {
  // Say this node is 013 (1011), mask is 077 or (00111111)
  // Say we want to use pipe 3 (11)
  // 6 bits in node mask, so shift pipeNo 6 times left and | into address
  uint16_t m = node_mask >> 3;
  uint8_t i = 0;

  while (m) { // While there are bits left in the node mask
    m >>= 1;  // Shift to the right
    i++;      // Count the # of increments
  }
  return node | (pipeNo << i);
}

/******************************************************************/

uint16_t RF24Network::direct_child_route_to(uint16_t node) {
  // Presumes that this is in fact a child!!
  uint16_t child_mask = (node_mask << 3) | 0x07;
  return node & child_mask;
}

/******************************************************************/

bool RF24Network::is_valid_address(uint16_t node) {
  bool result = true;
  if (node == 0100 || node == 010) {
    return result;
  }
  uint8_t count = 0;
  while (node) {
    uint8_t digit = node & 0x07;
    if (digit < 1 || digit > 5) {
      result = false;
      break;
    }
    node >>= 3;
    count++;
  }

  if (count > 4) {
    return false;
  }
  return result;
}

/******************************************************************/
#if defined(RF24NetworkMulticast)
void RF24Network::multicastLevel(uint8_t level) {
  multicast_level = level;
  radio.stopListening();
  radio.openReadingPipe(0, pipe_address(levelToAddress(level), 0));
  radio.startListening();
}

/******************************************************************/

uint16_t RF24Network::levelToAddress(uint8_t level) {

  uint16_t levelAddr = 1;
  if (level) {
    levelAddr = levelAddr << ((level - 1) * 3);
  } else {
    return 0;
  }
  return levelAddr;
}
#endif
/******************************************************************/

uint64_t RF24Network::pipe_address(uint16_t node, uint8_t pipe) {

  static uint8_t address_translation[] = {0xc3, 0x3c, 0x33, 0xce,
                                          0x3e, 0xe3, 0xec};
  uint64_t result = 0xCCCCCCCCCCLL;
  uint8_t *out = reinterpret_cast<uint8_t *>(&result);

  // Translate the address to use our optimally chosen radio address bytes
  uint8_t count = 1;
  uint16_t dec = node;

  while (dec) {
#if defined(RF24NetworkMulticast)
    if (pipe != 0 || !node)
#endif
      out[count] = address_translation[(
          dec % 8)]; // Convert our decimal values to octal, translate them to
                     // address bytes, and set our address

    dec /= 8;
    count++;
  }

#if defined(RF24NetworkMulticast)
  if (pipe != 0 || !node)
#endif
    out[0] = address_translation[pipe];
#if defined(RF24NetworkMulticast)
  else
    out[1] = address_translation[count - 1];
#endif

  return result;
}
