import json
from uuid import uuid4

from RF24 import RF24, RF24_2MBPS, RF24_CRC_16, RF24_PA_MAX


def _read_config():
    with open("settings.json") as file:
        return json.load(file)


def _save_config(values):
    with open("settings.json", mode="wt") as file:
        return json.dump(values, file)


def get_gateway():
    config = _read_config()

    radio = RF24(config["hub"]["ce_pin"], config["hub"]["csn_pin"])
    if not radio.begin():
        raise RuntimeError("radio cannot be initialized")

    radio.setPALevel(globals()[config["hub"]["power_level"]])
    radio.channel = config["hub"]["starting_channel"]

    radio.setDataRate(globals()[config["general"]["data_rate"]])
    radio.setCRCLength(globals()[config["general"]["crc_length"]])
    radio.address_width = config["general"]["address_width"]
    radio.payloadSize = config["general"]["payload_size"]

    if "gateway_id" not in config["hub"]:
        config["hub"]["gateway_id"] = uuid4().hex

    _save_config(config)

    return Gateway(radio, config)


class Gateway:
    def __init__(self, radio, config):
        self._radio = radio
        self._gateway_id = config["hub"]["gateway_id"]
        self._channel_range = config["general"]["channel_range"]
        self._config = config

    @property
    def gateway_id(self):
        return self._gateway_id

    @property
    def start_channel(self):
        return self._channel_range[0]

    @property
    def end_channel(self):
        return self._channel_range[1]

    @property
    def channel(self):
        return self._radio.channel

    @channel.setter
    def channel(self, ch):
        self._radio.channel = ch
        self._config["hub"]["starting_channel"] = ch
        _save_config(self._config)

    @property
    def is_pvariant(self):
        return self._radio.isPVariant()

    def check_signal(self):
        if self.is_pvariant:
            good_signal = self._radio.testRPD()
            if self._radio.available():
                self._radio.read(0, 0)
            return 1 if good_signal else 0
        return 1 if self._radio.testCarrier() else 0

    def start_listening(self):
        self._radio.startListening()

    def stop_listening(self):
        self._radio.stopListening()
