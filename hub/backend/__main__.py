from .data import init_db, update_master
from .gateway import get_gateway
from .scan import find_new_channel
import time


def main():
    print("Starting Gateway...")
    gateway = get_gateway()

    print("Initializing database...")
    init_db()
    update_master(gateway)

    print(f"Gateway {gateway.gateway_id} searching for new channel.")

    gateway.stop_listening()
    # channel = find_new_channel(gateway, 10)
    # gateway.channel = channel
    gateway.start()

    print(f"Gateway active on channel {gateway.channel}")
    print("Pipes")
    for pipe in gateway.children:
        print(hex(pipe))

    while True:

        pass


if __name__ == "__main__":
    main()
