import json
import os
from random import randint
from uuid import uuid4

from RF24 import RF24, RF24_2MBPS, RF24_CRC_16, RF24_PA_MAX

_SETTINGS_DIR = os.path.dirname(__file__)


def _read_config():
    with open(os.path.join(_SETTINGS_DIR, "settings.json")) as file:
        return json.load(file)


def _save_config(values):
    with open(os.path.join(_SETTINGS_DIR, "settings.json"), mode="wt") as file:
        return json.dump(values, file)


def _generate_device_id():
    return uuid4().hex


def _generate_address(prefix, number):
    return (prefix << 1) | number


def _generate_transceiver_address():
    return randint(0, 0xFFFFFFFFFF)


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

    radio.enableDynamicPayloads()
    radio.setAutoAck(1)
    radio.setAutoAck(0, 0)

    if "gateway_id" not in config["hub"]:
        config["hub"]["gateway_id"] = _generate_device_id()

    _save_config(config)

    return Gateway(radio, config)


class Gateway:
    def __init__(self, radio, config):
        self._radio = radio
        self._gateway_id = config["hub"]["gateway_id"]
        self._channel_range = config["general"]["channel_range"]
        self._config = config

        self._node_address = 0
        self._multicast_level = 0
        self._parent_node = -1
        self._parent_pipe = 0
        self._node_mask = 0
        self._pipe_addresses = [0] * 6

    def start(self):
        retry_var = (((self._node_address % 6) + 1) * 2) + 3
        self._radio.setRetries(retry_var, 5)
        self.setup_address()
        i = 6
        while i:
            i -= 1
            self._pipe_addresses[i] = self.pipe_address(self._node_address, i)
            self._radio.openReadingPipe(i, self._pipe_addresses[i])

        self.start_listening()

    @property
    def gateway_id(self):
        return self._gateway_id

    def setup_address(self):
        node_mask_check = 0xFFFF
        count = 0
        while self._node_address & node_mask_check:
            node_mask_check <<= 3
            count += 1
        self._multicast_level = count
        self._node_mask = ~node_mask_check & 0xFFFF
        parent_mask = self._node_mask >> 3
        self._parent_node = self._node_address & parent_mask
        i = self._node_address
        m = parent_mask
        while m:
            i >>= 3
            m >>= 3
        self._parent_pipe = i

    def pipe_address(self, node, pipe):
        address_translation = [0xc3, 0x3c, 0x33, 0xce, 0x3e, 0xe3, 0xec]
        out = list(0xCCCCCCCCCC000000.to_bytes(8, byteorder='big', signed=False))
        count = 1
        dec = node
        while dec:
            if pipe != 0 or node == 0:
                out[count] = address_translation[dec % 8]
            dec = int(dec/8)
            count += 1

        if pipe != 0 or node == 0:
            out[0] = address_translation[pipe]
        else:
            out[1] = address_translation[count - 1]

        return int.from_bytes(out, byteorder='big', signed=False)

    @property
    def address(self):
        return self._node_address

    @property
    def children(self):
        return self._pipe_addresses

    @property
    def parent(self):
        return 0 if self._node_address == 0 else self._parent_node

    def regenerate_gateway_id(self):
        self._gateway_id = _generate_device_id()
        self._config["hub"]["gateway_id"] = self._gateway_id
        _save_config(self._config)

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

# E3CCCCCCCC000 -- -0
# 3ECCCCCCCC000
# CECCCCCCCC000
# 33CCCCCCCC000
# 3CCCCCCCCC000
# C3CCCCCCCC000

# E33E3E3E3E000 -- default
# 3E3E3E3E3E000
# CE3E3E3E3E000
# 333E3E3E3E000
# 3C3E3E3E3E000
# CC3ECCCCCC000
