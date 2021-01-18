from RF24 import RF24, RF24_PA_MAX, RF24_2MBPS, RF24_CRC_16

hub_power_level = RF24_PA_MAX
hub_data_rate = RF24_2MBPS
hub_crc_length = RF24_CRC_16

payload_size = 128
address_width = 5

ce_pin = 22
csn_pin = 0


def get_radio():
    radio = RF24(ce_pin, csn_pin)
    if not radio.begin():
        raise RuntimeError("radio cannot be initialized")
    radio.address_width = address_width
    return radio

radio = get_radio()