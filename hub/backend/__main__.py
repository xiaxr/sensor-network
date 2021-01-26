import signal
import sys
from time import sleep

from config import SENSOR_LOG_FILE
from gateway import initialize_gateway
from network import MEASUREMENT_MESSAGE_TYPE


def terminateProcess(signalNumber, frame):
    sys.exit()

def append_data(msg):
    with open(SENSOR_LOG_FILE, mode='at') as file:
        file.write(msg.to_json())
        file.write('\n')

def handler(gateway):
    while True:
        for msg in gateway.next():
            if msg.message_type == MEASUREMENT_MESSAGE_TYPE:
                append_data(msg)
        sleep(0.5)

def main():
    signal.signal(signal.SIGTERM, terminateProcess)

    print("Starting Gateway...")
    gateway = initialize_gateway()

    gateway.begin()
    print(f"Gateway active on channel {gateway.channel}")

    try:
        handler(gateway)
    except KeyboardInterrupt:
        pass

    gateway.power_down()


if __name__ == "__main__":
    main()
