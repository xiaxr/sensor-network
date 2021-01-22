from .data import init_db, new_device_entry
from .gateway import initialize_gateway

def main():
    print("Starting Gateway...")
    gateway = initialize_gateway()

    print("Initializing database...")
    init_db()
    gateway.update_device_id(
        new_device_entry(gateway.device.device_id, 0, gateway.device.name, ""))

    gateway.begin()

    print(f"Gateway active on channel {gateway.channel}")

    while True:
        if gateway.update():
            while gateway.avaliable:
                frame = gateway.next()
                print(frame)


if __name__ == "__main__":
    main()
