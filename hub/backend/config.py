import RF24

import json


def _read_config():
    with open("settings.json") as file:
        return json.load(file)


def get_radio():
    config = _read_config()

    radio = RF24.RF24(config["hub"]["ce_pin"], config["hub"]["csn_pin"])
    if not radio.begin():
        raise RuntimeError("radio cannot be initialized")

    radio.power_level = globals()[config["hub"]["power_level"]]
    radio.channel = config["hub"]["starting_channel"]

    radio.data_rate = globals()[config["hub"]["data_rate"]]
    radio.crc_length = globals()[config["hub"]["crc_length"]]
    radio.address_width = config["general"]["address_width"]
    radio.payload_size = config["general"]["payload_size"]

    return radio

radio = get_radio()