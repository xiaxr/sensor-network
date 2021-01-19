from .data import init_db, update_master
from .gateway import get_gateway
from .scan import find_new_channel


def main():
    print("Starting Gateway...")
    gateway = get_gateway()

    print("Initializing database...")
    init_db()
    update_master(gateway)

    print(
        f"Gateway {gateway.gateway_id} [{gateway.gateway_address}] searching for new channel."
    )

    gateway.channel = find_new_channel(gateway, 10)
    print(f"Gateway active on channel {gateway.channel}")

    while True:
        pass


if __name__ == "__main__":
    main()
