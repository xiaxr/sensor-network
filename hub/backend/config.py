import json
import os.path
from typing import NamedTuple, Optional

from RF24 import RF24_2MBPS, RF24_PA_MAX, RF24_CRC_16

_SETTINGS_PATH = os.path.join( os.path.dirname(__file__),"data")

_SETTINGS_FILE = os.path.join(_SETTINGS_PATH, "gateway.json")
_DEFAULT_NETWORK_CHANNEL = 90
_DEFAULT_CE_PIN = 22
_DEFAULT_CSN_PIN = 0

SENSOR_LOG_FILE =  os.path.join(_SETTINGS_PATH, "data.log")

NETWORK_DATA_RATE = RF24_2MBPS
NETWORK_CRC_LENGTH = RF24_CRC_16
NETWORK_ADDRESS_WIDTH = 5

GATEWAY_NAME = "xiaxr hub"
GATEWAY_PA_LEVEL = RF24_PA_MAX

DEFAULT_NETWORK_ADDRESS = 0o4444

class GatewayConfiguration(NamedTuple):
    network_channel: int
    gateway_device_id: Optional[str]
    ce_pin: int
    csn_pin: int

    def save(self):
        with open(_SETTINGS_FILE, mode="wt") as file:
            json.dump(
                {
                    "network_channel": self.network_channel,
                    "gateway_device_id": self.gateway_device_id,
                    "ce_pin": self.ce_pin,
                    "csn_pin": self.csn_pin,
                }, file)

    @classmethod
    def load(cls):
        if os.path.exists(_SETTINGS_FILE):
            with open(_SETTINGS_FILE, mode="rt") as file:
                values = json.load(file)
                return cls(
                    values.get("network_channel", _DEFAULT_NETWORK_CHANNEL),
                    values.get("gateway_device_id"),
                    values.get("ce_pin", _DEFAULT_CE_PIN),
                    values.get("csn_pin", _DEFAULT_CSN_PIN),
                )
        return cls(_DEFAULT_NETWORK_CHANNEL, None, _DEFAULT_CE_PIN,
                   _DEFAULT_CSN_PIN)

    @classmethod
    def update_network_channel(cls, network_channel):
        config = cls.load()
        cls(network_channel, config.gateway_device_id, config.ce_pin,
            config.csn_pin).save()

    @classmethod
    def update_gateway_device_id(cls, gateway_device_id):
        config = cls.load()
        cls(config.network_channel, gateway_device_id, config.ce_pin,
            config.csn_pin).save()
