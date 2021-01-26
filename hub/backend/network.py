from datetime import datetime
import json
import struct
from typing import NamedTuple, Union

# device id (16 bytes) / timestamp (uint32_t) / message type / message length
_NETWORK_MESSAGE_HEADER = struct.Struct("<16BIBB")
# measurement id / measurement type / measurement unit / measurement length
_MEASUREMENT_MESSAGE_PAYLOAD = struct.Struct("<BBBB")

MEASUREMENT_MESSAGE_TYPE = 0x10


def decode_network_message(frame):
    if len(frame) < _NETWORK_MESSAGE_HEADER.size:
        return None

    header = NetworkMessageHeader.from_bytes(
        frame[:_NETWORK_MESSAGE_HEADER.size])

    if len(frame) < header.message_length:
        return None

    if header.message_type == MEASUREMENT_MESSAGE_TYPE:
        measurement = MeasurementMessage.from_bytes(
            header, frame[_NETWORK_MESSAGE_HEADER.size:])
        return measurement

    return None


class NetworkMessageHeader(NamedTuple):
    device_id: str
    timestamp: int
    message_type: int
    message_length: int
    recieved: int

    @classmethod
    def from_bytes(cls, value):
        unpacked = _NETWORK_MESSAGE_HEADER.unpack(value)
        return cls(
            bytes(unpacked[:16]).hex().upper(), *unpacked[16:],
            datetime.utcnow().timestamp())


class MeasurementMessage(NamedTuple):
    header: NetworkMessageHeader
    measurement_id: int
    measurement_type: int
    measurement_unit: int
    value: Union[int, bool, float]

    @classmethod
    def from_bytes(cls, header, value):
        measurement_id, measurement_type, measurement_unit, measurement_length = _MEASUREMENT_MESSAGE_PAYLOAD.unpack(
            value[:_MEASUREMENT_MESSAGE_PAYLOAD.size])
        is_float = measurement_length & 0x1
        measurement_length >>= 4
        measurement_value = (0, )
        if measurement_length == 1:
            measurement_value = value[_MEASUREMENT_MESSAGE_PAYLOAD.size]
        elif measurement_length == 2:
            measurement_value = struct.unpack(
                "<H", value[_MEASUREMENT_MESSAGE_PAYLOAD.size:(
                    _MEASUREMENT_MESSAGE_PAYLOAD.size + 2)])
        elif measurement_length == 4:
            if is_float:
                measurement_value = struct.unpack(
                    "<f", value[_MEASUREMENT_MESSAGE_PAYLOAD.size:(
                        _MEASUREMENT_MESSAGE_PAYLOAD.size + 4)])
            else:
                measurement_value = struct.unpack(
                    "<I", value[_MEASUREMENT_MESSAGE_PAYLOAD.size:(
                        _MEASUREMENT_MESSAGE_PAYLOAD.size + 4)])
        elif measurement_length == 8:
            if is_float:
                measurement_value = struct.unpack(
                    "<d", value[_MEASUREMENT_MESSAGE_PAYLOAD.size:(
                        _MEASUREMENT_MESSAGE_PAYLOAD.size + 8)])
            else:
                measurement_value = struct.unpack(
                    "<Q", value[_MEASUREMENT_MESSAGE_PAYLOAD.size:(
                        _MEASUREMENT_MESSAGE_PAYLOAD.size + 8)])
        return cls(header, measurement_id, measurement_type, measurement_unit,
                   measurement_value[0])

    @property
    def message_type(self):
        return self.header.message_type

    def to_json(self):
        return json.dumps(
            dict(device_id=self.header.device_id,
                 timestamp=self.header.recieved,
                 sequence=self.header.timestamp,
                 measurement_id=self.measurement_id,
                 measurement_type=self.measurement_type,
                 measurement_unit=self.measurement_unit,
                 value=self.value))
