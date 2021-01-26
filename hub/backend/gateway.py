from time import sleep

from RF24 import RF24
from rf24netx import RF24Network

from config import (GATEWAY_NAME, GATEWAY_PA_LEVEL, 
                     NETWORK_ADDRESS_WIDTH, NETWORK_CRC_LENGTH,
                     NETWORK_DATA_RATE, GatewayConfiguration)
from device import Device
from network import decode_network_message

def initialize_gateway():
    config = GatewayConfiguration.load()
    return Gateway(RF24(config.ce_pin, config.csn_pin), config.network_channel,
                   config.gateway_device_id)


class Gateway:
    def __init__(self, radio, channel, device_id):
        self._radio = radio
        self._network = RF24Network(self._radio)
        self._device = Device(GATEWAY_NAME, device_id=device_id)
        self._channel = channel

    def begin(self):
        if not self._radio.begin():
            raise RuntimeError("cannot start radio")
        self._radio.setPALevel(GATEWAY_PA_LEVEL)

        self._radio.setAddressWidth(NETWORK_ADDRESS_WIDTH)
        self._radio.setCRCLength(NETWORK_CRC_LENGTH)
        self._radio.setDataRate(NETWORK_DATA_RATE)

        self._radio.powerUp()
        sleep(0.1)
        self._network.begin(self._channel, 0)

    @property
    def device(self):
        return self._device

    def update_device_id(self, value):
        self._device.device_id = value
        GatewayConfiguration.update_gateway_device_id(value)

    @property
    def network(self):
        return self._network

    def update_channel(self, value):
        self._channel = value
        self._radio.stopListening()
        self._radio.setChannel(value)
        sleep(0.2)
        self._radio.startListening()
        GatewayConfiguration.update_network_channel(value)

    @property
    def channel(self):
        return self._channel

    def next(self):
        frames = self._network.next()        
        out = []
        for frame in frames:
            msg = decode_network_message(frame)            
            if msg is not None:
                out.append(msg)
        return out

    def power_down(self):
        self._radio.powerDown()

    def power_up(self):
        self._radio.powerUp()
