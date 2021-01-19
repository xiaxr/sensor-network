import struct
from enum import IntEnum

from typing import NamedTuple, Any, Sequence
from .device import DeviceAddress

_MESSAGE_ID = "H"
_DEVICE_ID = "16B"
_DEVICE_ADDRESS = "Q"
_MESSAGE_PREAMBLE = _MESSAGE_ID + _DEVICE_ID + _DEVICE_ADDRESS + _DEVICE_ID + _DEVICE_ADDRESS
_MESSAGE_BODY = "B" * (128 - struct.calcsize(_MESSAGE_PREAMBLE))
_MESSAGE_BODY_SIZE = struct.calcsize(_MESSAGE_BODY)
_message_encoded = struct.Struct("!" + _MESSAGE_PREAMBLE + _MESSAGE_BODY)

_master_advertise_encoded = struct.Struct("!Hd")  # channel + timestamp


class MessageID(IntEnum):
    PingRequest = 1
    PingResponse = 2

    MasterAdvertise = 4

    NetworkJoinRequest = 10
    NewAddressRequest = 11
    NewAddressResponse = 12
    NewAddressAccepted = 13

    DeviceIDRequest = 20
    DeviceIDResponse = 21
    DeviceDescriptionRequest = 22
    DeviceDescriotionResponse = 23
    DeviceDescriptionAccepted = 24

    MeasurementSend = 50
    MeasurementRequest = 51
    MeasurementDescriptionRequest = 52
    MeasurementDescriptionResponse = 53


_MESSAGE_BODY_PACKERS = {
    MessageID.PingRequest: None,
    MessageID.PingRequest: None,
    MessageID.MasterAdvertise: _master_advertise_encoded,
    MessageID.NetworkJoinRequest: None,
    MessageID.NewAddressRequest: None,
    MessageID.NewAddressResponse: None,
    MessageID.NewAddressAccepted: None,
    MessageID.DeviceIDRequest: None,
    MessageID.DeviceIDResponse: None,
    MessageID.DeviceDescriptionRequest: None,
    MessageID.DeviceDescriotionResponse: None,
    MessageID.DeviceDescriptionAccepted: None,
    MessageID.MeasurementSend: None,
    MessageID.MeasurementRequest: None,
    MessageID.MeasurementDescriptionRequest: None,
    MessageID.MeasurementDescriptionResponse: None,
}


class Message(NamedTuple):
    message_id: MessageID
    sender_device_address: DeviceAddress
    recipient_device_address: DeviceAddress
    body: Sequence[Any]

    def package_message(self):
        body = bytes(_MESSAGE_BODY_SIZE)
        body_encoder = _MESSAGE_BODY_PACKERS[self.message_id]
        if body_encoder is not None:
            body_encoder.pack_into(body, 0, *self.body)

        _message_encoded.pack(int(self.message_id),
                              self.sender_device_address.device_id,
                              self.sender_device_address.device_address,
                              self.recipient_device_address.device_id, body)

    @classmethod
    def _new_message(cls, message_id, sender_device_address,
                     recipient_device_address, *body):
        return cls(message_id, sender_device_address, recipient_device_address,
                   *body)

    @classmethod
    def new_ping_request(cls, sender_device_address, recipient_device_address):
        return cls._new_message(MessageID.PingRequest, sender_device_address,
                                recipient_device_address)

    @classmethod
    def new_ping_response(cls, sender_device_address,
                          recipient_device_address):
        return cls._new_message(MessageID.PingResponse, sender_device_address,
                                recipient_device_address)

    @classmethod
    def new_master_advertise(cls, sender_device_address,
                             recipient_device_address, channel, timestamp):
        return cls._new_message(MessageID.MasterAdvertise,
                                sender_device_address,
                                recipient_device_address, channel, timestamp)
