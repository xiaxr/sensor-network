from RF24 import RF24, RF24_2MBPS, RF24_CRC_16, RF24_PA_MAX

import json


def _read_config():
    with open("settings.json") as file:
        return json.load(file)


def get_radio():
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

    return Gateway(radio, config["general"]["channel_range"])


class Gateway:
    def __init__(self, radio, channel_range):
        self._radio = radio
        self._channel_range = channel_range

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
        self._radio = ch

    @property
    def is_pvariant(self):
        return self._radio.isPVariant()

    def check_signal(self):
        if self.is_pvariant:
            good_signal = self._radio.testRPD()
            if self._radio.available():
                self._radio.read(0, 0)
            return 1 if good_signal else 0
        return 0 if self._radio.testCarrier() else 1

    def start_listening(self):
        self._radio.startListening()
    
    def stop_listening(self):
        self._radio.stopListening()
    