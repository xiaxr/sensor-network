from RF24 import RF24

from .config import (GATEWAY_PA_LEVEL, HUB_NAME, NETWORK_ADDRESS_WIDTH,
                     NETWORK_CRC_LENGTH, NETWORK_DATA_RATE,
                     GatewayConfiguration)
from .device import Device
from .network import Network, MessageType
from .data import new_device_entry
import struct


def initialize_gateway():
    config = GatewayConfiguration.load()
    return Gateway(RF24(config.ce_pin, config.csn_pin), config.network_channel,
                   config.gateway_device_id)


class Gateway:
    def __init__(self, radio, channel, device_id):
        self._radio = radio
        self._network = Network(radio, 0)
        self._device = Device(HUB_NAME, device_id=device_id)
        self._channel = channel

    def begin(self):
        if not self._radio.begin():
            raise RuntimeError("could not initialize radio")

        self._radio.setChannel(self._channel)
        self._radio.setDataRate(NETWORK_DATA_RATE)
        self._radio.setPALevel(GATEWAY_PA_LEVEL)
        self._radio.setCRCLength(NETWORK_CRC_LENGTH)
        self._radio.setAddressWidth(NETWORK_ADDRESS_WIDTH)

        return self._network.begin()

    @property
    def device(self):
        return self._device

    def update_device_id(self, value):
        self._device.device_id = value
        GatewayConfiguration.update_gateway_device_id(value)

    @property
    def network(self):
        return self._network

    def update_network_channel(self, value):
        self._channel = value
        self._network.update_channel(value)
        GatewayConfiguration.update_network_channel(value)

    @property
    def channel(self):
        return self._channel

    def update(self):
        self._network.update()
        return self._network.available

    @property
    def avaliable(self):
        return self._network.available

    def next(self):
        frame = self._network.read()
        
        if frame.message_type == MessageType.RequestDeviceID:
            self._process_device_request(frame)

        return frame

    def _process_device_request(self, frame):
        noice_length = frame.value[0]
        noice = frame.value[1:(noice_length + 1)]
        device_id = new_device_entry(0, 0xFFFF, "", "")
        print(f"DeviceID Request from {frame.header.from_node}")
        print(f"noice {noice.hex()}")
        print(f"device_id {device_id}")

        self._network.broadcast(
            frame.encode_response(
                MessageType.DeviceID,
                struct.pack(
                    "<B" + ("B" * len(noice)) + ("B" * (len(device_id) / 2)),
                    noice_length, *noice, *bytes.fromhex(device_id))))
