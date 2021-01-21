from .data import init_db, new_device_entry
from .gateway import initialize_gateway
from RF24 import RF24
# from .scan import find_new_channel
# import time


def main():
    radio = RF24(22, 0)
    if not radio.begin():
        raise RuntimeError("cannot access radio")

    radio.setChannel(75)

    for idx, addr in enumerate([
            0x7878787878,
            0xB3B4B5B6F1,
            0xB3B4B5B6CD,
            0xB3B4B5B6A3,
            0xB3B4B5B60F,
            0xB3B4B5B605,
    ]):
        radio.openReadingPipe(idx, addr)

    radio.startListening()

    while True:
        print(radio.avaliable())
        if radio.available():
            data = radio.read(radio.getPayloadSize())
            print(data)

    # print("Starting Gateway...")
    # gateway = initialize_gateway()

    # print("Initializing database...")
    # init_db()
    # gateway.update_device_id(
    #     new_device_entry(gateway.device.device_id, 0, gateway.device.name, ""))

    # # print(f"Gateway {gateway.gateway_id} searching for new channel.")

    # # gateway.stop_listening()
    # # # channel = find_new_channel(gateway, 10)
    # # # gateway.channel = channel
    # gateway.begin()

    # print(f"Gateway active on channel {gateway.channel}")
    # print("Pipes Open:")
    # for pipe in gateway.pipe_addresses():
    #     print(hex(pipe).upper())

    # while True:
    #     if gateway.update():
    #         while gateway.avaliable:
    #             frame = gateway.next()
    #             print(frame)


if __name__ == "__main__":
    main()
