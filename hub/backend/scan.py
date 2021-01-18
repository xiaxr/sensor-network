from config import get_radio

import time


class Scanner:
    def __init__(self, radio):
        self._radio = radio

    def find_new_channel(self, repeats):
        values = {}
        for i in range(repeats):            
            for channel in range(self._radio.start_channel,
                                 self._radio.end_channel + 1):
                print(f"Interation {i+1} of {repeats} -- Channel {channel} of [{self._radio.start_channel}-{self._radio.end_channel + 1}]             \r",
                  end='',
                  flush=True)

                self._radio.channel = channel
                self._radio.start_listening()
                time.sleep(0.10)
                self._radio.stop_listening()
                if channel not in values:
                    values[channel] = 0
                values[channel] += self._radio.check_signal()

        return sorted(tuple(values.items()), key=lambda x: x[1])


if __name__ == "__main__":
    scanner = Scanner(get_radio())
    print(scanner.find_new_channel(10))