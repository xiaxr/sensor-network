from collections import deque
from enum import IntEnum
from struct import Struct
from time import monotonic, sleep
from typing import NamedTuple

from .config import (DEFAULT_NETWORK_ADDRESS, MAX_PAYLOAD_SIZE,
                     MULTICAST_DELAY_FACTOR, ROUTE_TIMEOUT_FACTOR, TX_TIMEOUT)


def level_to_address(level):
    if level:
        return 1 << ((level - 1) * 3)
    return 0


def pipe_address(node, pipe):
    address_translation = [0xc3, 0x3c, 0x33, 0xce, 0x3e, 0xe3, 0xec]
    out = list(0xCCCCCCCCCC000000.to_bytes(8, byteorder='big', signed=False))
    count = 1
    dec = node
    while dec:
        if pipe != 0 or node == 0:
            out[count] = address_translation[dec % 8]
        dec = int(dec / 8)
        count += 1

    if pipe != 0 or node == 0:
        out[0] = address_translation[pipe]
    else:
        out[1] = address_translation[count - 1]

    return int.from_bytes(out, byteorder='big', signed=False)


def is_valid_address(node):
    if node in (0o100, 0o10):
        return True

    count = 0
    while node:
        digit = node & 0x07
        if digit < 1 or digit > 5:
            return False
        node >>= 3
        count += 1

    return count < 4


class MessageType(IntEnum):
    Ping = 1


class RoutingMode(IntEnum):
    Normal = 0
    Routed = 1
    UserToPhysicalAddress = 2
    UserToLogicalAddress = 3
    Multicast = 4


class Routing(NamedTuple):
    to_node: int
    send_pipe: RoutingMode
    multicast: bool


ENCODED_NETWORK_HEADER = Struct("!HHB")


class NetworkHeader(NamedTuple):
    from_node: int
    to_node: int
    message_type: MessageType

    def encode(self):
        return ENCODED_NETWORK_HEADER.pack(self.from_node, self.to_node,
                                           self.message_type)

    @classmethod
    def decode(cls, raw_values):
        return cls(ENCODED_NETWORK_HEADER.unpack(raw_values))


class NetworkFrame:
    def __init__(self, header, value):
        self._header = header
        self._value = value

    @property
    def header(self):
        return self._header

    @property
    def value(self):
        return self._value

    def encode(self):
        if ENCODED_NETWORK_HEADER.size + len(self._value) > MAX_PAYLOAD_SIZE:
            raise RuntimeError("packet size too big")
        return self._header.encode() + self._value

    @classmethod
    def decode(cls, raw_values):
        return cls(
            NetworkHeader.decode(raw_values[:ENCODED_NETWORK_HEADER.size]),
            raw_values[ENCODED_NETWORK_HEADER.size:])


class Network:
    def __init__(self, radio, node_address):
        if not is_valid_address(node_address):
            raise RuntimeError(f"{node_address} is not a valid node address")

        self._radio = radio
        self._node_address = node_address
        self._multicast_level = 0
        self._node_mask = 0
        self._parent_pipe = 0
        self._parent_node = 0
        self._route_timeout = 0
        self._tx_timeout = 0
        self._frame_stack = deque()

    def _setup_address(self):
        node_mask_check = 0xFFFF
        count = 0
        while self._node_address & node_mask_check:
            node_mask_check <<= 3
            count += 1
        self._multicast_level = count
        self._node_mask = ~node_mask_check & 0xFFFF
        parent_mask = self._node_mask >> 3
        self._parent_node = self._node_address & parent_mask
        i = self._node_address
        m = parent_mask
        while m:
            i >>= 3
            m >>= 3
        self._parent_pipe = i

    def multicast_level(self, level):
        self._multicast_level = level
        self._radio.stopListening()
        self._radio.openReadingPipe(0, pipe_address(level_to_address(level),
                                                    0))
        self._radio.startListening()

    def begin(self):
        if not self._radio.isValid():
            raise RuntimeError("error communicating with radio")

        self._radio.setAutoAck(1)
        self._radio.setAutoAck(0, 0)  # disable auto auto ack on multicast pipe

        self._radio.enableDynamicPayloads()

        # Use different retry periods to reduce data collisions as per datasheet
        self._radio.setRetries((((self._node_address % 6) + 1) * 2) + 3, 5)
        self._tx_timeout = TX_TIMEOUT
        self._route_timeout = self._tx_timeout * ROUTE_TIMEOUT_FACTOR

        self._setup_address()

        i = 6
        while i:
            i -= 1
        self._radio.openReadingPipe(i, pipe_address(self._node_address, i))

        self._radio.startListening()
        return True

    @property
    def node_address(self):
        return self._node_address

    def pipe_addresses(self):
        return [pipe_address(self._node_address, i) for i in range(1, 7)]

    def update_channel(self, channel):
        self._radio.stopListening()
        self._radio.setChannel(channel)
        self._radio.startListening()

    @property
    def available(self):
        return bool(self._frame_stack)

    def update(self):
        timeout = monotonic()
        while self._radio.available():
            if monotonic() - timeout >= 1:
                break

            frame_size = self._radio.getDynamicPayloadSize()
            encoded_frame = self._radio.read(frame_size)
            if len(frame_size) < ENCODED_NETWORK_HEADER.size:
                continue
            frame = NetworkFrame.decode(encoded_frame)
            if not (is_valid_address(frame.header.to_node)
                    and is_valid_address(frame.header.from_node)
                    ) or frame.header.from_node == self._node_address:
                continue

            if frame.header.to_node == self._node_address:
                if frame.header.message_type == MessageType.Ping:  # ignore auto ack repings
                    continue
                self._frame_stack.append(frame)
            elif frame.header.to_node == 0o100:
                if self._node_address >> 3:
                    sleep(MULTICAST_DELAY_FACTOR * 4)
                sleep(MULTICAST_DELAY_FACTOR * (self._node_address % 4))
                self._write(encoded_frame,
                            level_to_address(self._multicast_level) << 3,
                            RoutingMode.Multicast)
            elif frame.header.to_node != DEFAULT_NETWORK_ADDRESS:
                self._write(encoded_frame, frame.header.to_node,
                            RoutingMode.Routed)

    def _write_to_pipe(self, encoded_frame, to_node, pipe, multicast):
        self._radio.stopListening()
        if multicast:
            self._radio.setAutoAck(0, 0)
        else:
            self._radio.setAutoAck(0, 1)

        self._radio.openWritingPipe(pipe_address(to_node, pipe))
        self._radio.writeFast(encoded_frame, False)
        ok = self._radio.txStandBy(self._tx_timeout)
        self._radio.setAutoAck(0, 0)

        return ok

    def _write(self, encoded_frame, to_node, routing_mode):
        route = self._get_routing(to_node, routing_mode)
        if route is None:
            return False
        if routing_mode == RoutingMode.Routed and route.to_node == to_node:
            sleep(2 / 1000)
        ok = self._write_to_pipe(encoded_frame, route.to_node, route.send_pipe,
                                 route.multicast)

        # insert possible code for network acking
        # if( directTo == TX_ROUTED && ok && conversion.send_node == to_node && isAckType){

        # RF24NetworkHeader* header = (RF24NetworkHeader*)&frame_buffer;
        # header->type = NETWORK_ACK;				    // Set the payload type to NETWORK_ACK
        # header->to_node = header->from_node;          // Change the 'to' address to the 'from' address

        # conversion.send_node = header->from_node;
        # conversion.send_pipe = TX_ROUTED;
        # conversion.multicast = 0;
        # logicalToPhysicalAddress(&conversion);

        # //Write the data using the resulting physical address
        # frame_size = sizeof(RF24NetworkHeader);
        # write_to_pipe(conversion.send_node, conversion.send_pipe, conversion.multicast);

        # if( ok && conversion.send_node != to_node && (directTo==0 || directTo==3) && isAckType){
        # // Now, continue listening
        # if(networkFlags & FLAG_FAST_FRAG){
        # radio.txStandBy(txTimeout);
        # networkFlags &= ~FLAG_FAST_FRAG;
        # radio.setAutoAck(0,0);
        # }
        # radio.startListening();
        # uint32_t reply_time = millis();

        # while( update() != NETWORK_ACK){
        # #if defined (RF24_LINUX)
        # delayMicroseconds(900);
        # #endif
        # if(millis() - reply_time > routeTimeout){
        # #if defined (RF24_LINUX)
        # IF_SERIAL_DEBUG_ROUTING( printf_P(PSTR("%u: MAC Network ACK fail from 0%o via 0%o on pipe %x\n\r"),millis(),to_node,conversion.send_node,conversion.send_pipe); );
        # #else
        # IF_SERIAL_DEBUG_ROUTING( printf_P(PSTR("%lu: MAC Network ACK fail from 0%o via 0%o on pipe %x\n\r"),millis(),to_node,conversion.send_node,conversion.send_pipe); );
        # #endif
        # ok=false;
        # break;
        # }
        # }
        # }

        self._radio.startListening()

        return ok

    def multicast(self, message_type, payload, level):
        return self.write_direct(
            NetworkFrame(
                NetworkHeader(self._node_address, 0o100, message_type),
                payload).encode(), 0o100, level_to_address(level))

    def write(self, to_node, message_type, payload):
        return self.write_direct(
            NetworkFrame(
                NetworkHeader(self._node_address, to_node, message_type),
                payload).encode(), to_node, 0o70)

    def write_direct(self, encoded_frame, to_node, direct):
        if direct == 0o70:
            return self._write(encoded_frame, to_node, RoutingMode.Normal)

        if to_node == 0o100:
            return self._write(encoded_frame, direct, RoutingMode.Multicast)

        if to_node == direct:
            return self._write(encoded_frame, direct,
                               RoutingMode.UserToPhysicalAddress)

        return self._write(encoded_frame, direct,
                           RoutingMode.UserToLogicalAddress)

    def _get_routing(self, to_node, routing_mode):
        if routing_mode in (RoutingMode.UserToLogicalAddress,
                            RoutingMode.UserToPhysicalAddress,
                            RoutingMode.Multicast):
            return Routing(to_node, 0, True)

        if self.is_direct_child(to_node):
            return Routing(to_node, 5, False)

        if self.is_descendant(to_node):
            return Routing(self._direct_child_route_to(to_node), 5)

        if self._node_address:
            return Routing(self._parent_node, 0, False)

        return None

    def is_direct_child(self, node):
        # A direct child of ours has the same low numbers as us, and only
        # one higher number.
        # e.g. node 0234 is a direct child of 034, and node 01234 is a
        # descendant but not a direct child
        if not self.is_descendant(node):
            return False

        child_node_mask = (~self._node_mask & 0xFFFF) >> 3
        return (node & child_node_mask) == 0

    def is_descendant(self, node):
        return (node & self._node_mask) == self._node_address

    def _direct_child_route_to(self, node):
        child_mask = (self._node_mask << 3) | 0x07
        return node & child_mask

    def read(self):
        if self._frame_stack:
            return self._frame_stack.pop()
        return None
