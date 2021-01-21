from .data import init_db, new_device_entry
from .gateway import initialize_gateway

# from .scan import find_new_channel
# import time


def main():
    print("Starting Gateway...")
    gateway = initialize_gateway()

    print("Initializing database...")
    init_db()
    new_device_entry(gateway.device.device_id, 0, gateway.device.name, "")

    # print(f"Gateway {gateway.gateway_id} searching for new channel.")

    # gateway.stop_listening()
    # # channel = find_new_channel(gateway, 10)
    # # gateway.channel = channel
    gateway.begin()

    print(f"Gateway active on channel {gateway.channel}")

    while True:
        if gateway.update():
            while gateway.avaliable:
                frame = gateway.next()
                print(frame)


if __name__ == "__main__":
    main()
