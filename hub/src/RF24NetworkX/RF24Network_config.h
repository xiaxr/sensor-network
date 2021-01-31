/*
 Copyright (C) 2011 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#pragma once

#define NETWORK_DEFAULT_ADDRESS 04444

#define RF24NetworkMulticast

/* *
 * \def
 * Saves memory by disabling fragmentation
 */
//#define DISABLE_FRAGMENTATION

/** System defines */

/** Maximum size of fragmented network frames and fragmentation cache.
 *
 * @note: This buffer can now be any size > 24. Previously need to be a multiple
 * of 24.
 * @note: If used with RF24Ethernet, this value is used to set the buffer sizes.
 */
#define MAX_PAYLOAD_SIZE 144

/** The size of the main buffer. This is the user-cache, where incoming data is
 * stored. Data is stored using Frames: Header (8-bytes) + Frame_Size (2-bytes)
 * + Data (?-bytes)
 */
#define MAIN_BUFFER_SIZE (MAX_PAYLOAD_SIZE + FRAME_HEADER_SIZE)

/** Disable user payloads. Saves memory when used with RF24Ethernet or software
 * that uses external data.*/
#define DISABLE_USER_PAYLOADS

/** Enable tracking of success and failures for all transmissions, routed and
 * user initiated */
//#define ENABLE_NETWORK_STATS

/** Enable dynamic payloads - If using different types of NRF24L01 modules, some
 * may be incompatible when using this feature **/
#define ENABLE_DYNAMIC_PAYLOADS

#include <RF24X/RF24_config.h>
