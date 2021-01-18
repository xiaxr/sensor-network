from time import sleep
from config import get_gateway


class Scanner:
    def __init__(self, gateway):
        self._radio = gateway

    def find_new_channel(self, repeats):
        values = {}
        for i in range(repeats):
            for channel in range(self._radio.start_channel,
                                 self._radio.end_channel + 1):
                print(
                    f"Interation {i+1} of {repeats} -- Channel {channel} of [{self._radio.start_channel}-{self._radio.end_channel + 1}]             \r",
                    end='',
                    flush=True)

                self._radio.channel = channel
                self._radio.start_listening()
                sleep(0.10)
                self._radio.stop_listening()
                if channel not in values:
                    values[channel] = 0
                values[channel] += self._radio.check_signal()

        return sorted(tuple(values.items()), key=lambda x: x[1],
                      reverse=True)[0][0]


if __name__ == "__main__":
    gateway = get_gateway()
    print(f"Gateway - {gateway.gateway_id}")
    scanner = Scanner(gateway)
    new_channel = scanner.find_new_channel(10)
    print(f"New channel found {new_channel}")
    gateway.channel = new_channel
