from time import sleep


def find_new_channel(radio, repeats):
    values = {}
    for i in range(repeats):
        for channel in range(radio.start_channel, radio.end_channel + 1):
            print(
                f"Interation {i+1} of {repeats} -- Channel {channel} of [{radio.start_channel}-{radio.end_channel + 1}]             \r",
                end='',
                flush=True)

            radio.channel = channel
            radio.start_listening()
            sleep(0.10)
            radio.stop_listening()
            if channel not in values:
                values[channel] = 0
            values[channel] += radio.check_signal()
    print()
    return sorted(tuple(values.items()), key=lambda x: x[1],
                  reverse=True)[0][0]
