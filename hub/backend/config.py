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

    radio.setDataRate(globals()[config["hub"]["data_rate"]])
    radio.setCRCLength(globals()[config["hub"]["crc_length"]])
    radio.address_width = config["general"]["address_width"]
    radio.payloadSize = config["general"]["payload_size"]

    return radio


radio = get_radio()