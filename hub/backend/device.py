from enum import IntEnum
from typing import NamedTuple, Optional
from uuid import uuid4


def generate_device_id():
    return uuid4().hex


class DeviceType(IntEnum):
    Gateway = 0x00
    Generic = 0xFF


class DeviceAddress(NamedTuple):
    device_id: int
    device_address: int


class Device:
    def __init__(self, name: str, device_id: Optional[str] = None):
        if device_id is None or not device_id:
            self._device_id = generate_device_id()
        else:
            self._device_id = device_id
        self._name = name
        self._description = ""

    @property
    def device_id(self):
        return self._device_id

    @device_id.setter
    def device_id(self, value):
        self._device_id = value

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, value):
        self._name = value

    @property
    def description(self):
        return self._description

    @description.setter
    def description(self, value):
        self._description = value
