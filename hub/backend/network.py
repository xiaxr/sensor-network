from collections import deque
from enum import IntEnum
from struct import Struct
from time import monotonic, sleep
from typing import NamedTuple

from .config import (DEFAULT_NETWORK_ADDRESS, GATEWAY_MASTER_ADDRESS,
                     GATEWAY_BROADCAST_ADDRESS, MAX_PAYLOAD_SIZE,
                     MULTICAST_DELAY_FACTOR, ROUTE_TIMEOUT_FACTOR, TX_TIMEOUT)

# def level_to_address(level):
#     if level:
#         return 1 << ((level - 1) * 3)
#     return 0

# def pipe_address(node, pipe):
#     address_translation = [0xc3, 0x3c, 0x33, 0xce, 0x3e, 0xe3, 0xec]
#     out = list(0xCCCCCCCCCC000000.to_bytes(8, byteorder='big', signed=False))
#     count = 1
#     dec = node
#     while dec:
#         if pipe != 0 or node == 0:
#             out[count] = address_translation[dec % 8]
#         dec = int(dec / 8)
#         count += 1

#     if pipe != 0 or node == 0:
#         out[0] = address_translation[pipe]
#     else:
#         out[1] = address_translation[count - 1]

#     return int.from_bytes(out, byteorder='big', signed=False)

# def is_valid_address(node):
#     if node in (0o100, 0o10):
#         return True

#     count = 0
#     while node:
#         digit = node & 0x07
#         if digit < 1 or digit > 5:
#             return False
#         node >>= 3
#         count += 1

#     return count < 4


class MessageType(IntEnum):
    Ping = 0x01
    RequestDeviceID = 0x50
    DeviceID = 0x51

# class RoutingMode(IntEnum):
#     Normal = 0
#     Routed = 1
#     UserToPhysicalAddress = 2
#     UserToLogicalAddress = 3
#     Multicast = 4

# class Routing(NamedTuple):
#     to_node: int
#     send_pipe: RoutingMode
#     multicast: bool

ENCODED_NETWORK_HEADER = Struct("<HHBx")


class NetworkHeader(NamedTuple):
    from_node: int
    to_node: int
    message_type: MessageType

    def encode(self):
        return ENCODED_NETWORK_HEADER.pack(self.from_node, self.to_node,
                                           self.message_type)

    @classmethod
    def decode(cls, raw_values):
        return cls(*ENCODED_NETWORK_HEADER.unpack(raw_values))


class NetworkFrame:
    def __init__(self, header, value, pipe=-1):
        self._header = header
        self._value = value
        self._pipe = pipe

    @property
    def header(self):
        return self._header

    @property
    def message_type(self):
        return self._header.message_type

    @property
    def value(self):
        return self._value

    @property
    def pipe(self):
        return self._pipe

    def encode(self):
        if ENCODED_NETWORK_HEADER.size + len(self._value) > MAX_PAYLOAD_SIZE:
            raise RuntimeError("packet size too big")
        return self._header.encode() + self._value

    def encode_response(self, message_type, response):
        return NetworkFrame(NetworkHeader(self.header.to_node, self.header.from_node, message_type), response).encode()

    @classmethod
    def decode(cls, raw_values, pipe=-1):
        return cls(NetworkHeader.decode(
            raw_values[:ENCODED_NETWORK_HEADER.size]),
                   raw_values[ENCODED_NETWORK_HEADER.size:],
                   pipe=pipe)

    def __repr__(self):
        return f"NetworkFrame<{self._header.from_node:o}->{self._header.to_node:o}[{str(self._header.message_type)}]:{self._value}>"


class Network:
    def __init__(self, radio, node_address):
        self._radio = radio
        self._node_address = node_address
        self._frame_stack = deque()


    def begin(self):
        if not self._radio.isValid():
            raise RuntimeError("error communicating with radio")

        self._radio.setAutoAck(1)
        self._radio.setAutoAck(1, 0)  # disable auto auto ack on multicast pipe
        self._radio.enableDynamicPayloads()
        
        self._radio.setRetries(0, 0)

        self._radio.openReadingPipe(0, GATEWAY_MASTER_ADDRESS)
        self._radio.openReadingPipe(1, GATEWAY_BROADCAST_ADDRESS)

        self._radio.startListening()
        return True

    @property
    def node_address(self):
        return self._node_address

    def update_channel(self, channel):
        self._radio.stopListening()
        sleep(2 / 1000)
        self._radio.setChannel(channel)
        self._radio.startListening()

    @property
    def available(self):
        return bool(self._frame_stack)

    def update(self):
        timeout = monotonic()

        while True:
            pipe, ready = self._radio.available_pipe()
            if not ready:
                return

            if monotonic() - timeout >= 1:
                break

            frame_size = self._radio.getDynamicPayloadSize()
            encoded_frame = self._radio.read(frame_size)
            if frame_size < ENCODED_NETWORK_HEADER.size:
                continue
            
            frame = NetworkFrame.decode(encoded_frame, pipe=pipe)
            if frame.header.from_node == self._node_address or frame.header.to_node == frame.header.from_node:
                continue

            if frame.header.to_node == self._node_address:
                print(encoded_frame.hex())                
                self._frame_stack.append(frame)

    def broadcast(self, encoded_frame):
        self._radio.stopListening()
        sleep(2 / 1000)
        self._radio.setAutoAck(0, 0)
        self._radio.openWritingPipe(GATEWAY_BROADCAST_ADDRESS)
        self._radio.writeFast(encoded_frame, True)
        self._radio.txStandBy(TX_TIMEOUT)
        sleep(2 / 1000)
        self._radio.setAutoAck(0, 1)
        self._radio.startListening()

    def read(self):
        if self._frame_stack:
            return self._frame_stack.pop()
        return None
