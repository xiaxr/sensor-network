from intelhex import IntelHex
import argparse
from uuid import uuid4

_DEVICE_ID_OFFSET = 0
_DEVICE_ID_LENGTH = 16

_DEVICE_NAME_LENGTH_OFFSET = (_DEVICE_ID_OFFSET+_DEVICE_ID_LENGTH)
_DEVICE_NAME_LENGTH_LENGTH = 1

_DEVICE_NAME_OFFSET = (_DEVICE_NAME_LENGTH_OFFSET+_DEVICE_NAME_LENGTH_LENGTH)
_DEVICE_NAME_MAX_LENGTH = 32


def parse_args():
    parser = argparse.ArgumentParser(
        description='Generate device eeprom configuration.')
    parser.add_argument("--name", default="")
    parser.add_argument("--output", default="settings.hex")
    return parser.parse_args()


def main():
    args = parse_args()
    ih = IntelHex()
    
    ih.puts(_DEVICE_ID_OFFSET, uuid4().bytes)
    ih[_DEVICE_NAME_LENGTH_OFFSET] = min(len(args.name), _DEVICE_NAME_MAX_LENGTH)
    
    device_name = args.name.encode('utf-8')
    if len(device_name) <_DEVICE_NAME_MAX_LENGTH:
        device_name += bytes(_DEVICE_NAME_MAX_LENGTH-len(device_name))

    ih.puts(_DEVICE_NAME_OFFSET, device_name)

    ih.write_hex_file(args.output)


if __name__ == "__main__":
    main()