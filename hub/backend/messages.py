import struct

_MESSAGE_ID = "H"
_DEVICE_ID = "16B"
_DEVICE_ADDRESS = "Q"
_MESSAGE_PREAMBLE = _MESSAGE_ID+_DEVICE_ID +_DEVICE_ADDRESS+_DEVICE_ID +_DEVICE_ADDRESS
raise RuntimeError(struct.calcsize(_MESSAGE_PREAMBLE))
_MESSAGE_BODY = "B"* (128-struct.calcsize(_MESSAGE_PREAMBLE))

message_struct = struct.Struct("!"+_MESSAGE_PREAMBLE )