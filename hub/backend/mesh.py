def pipe_address(node, pipe):
    address_translation = [0xc3, 0x3c, 0x33, 0xce, 0x3e, 0xe3, 0xec]
    out = [0xBC, 0xBC, 0xBC, 0xBC, 0xBC]

    count = 1
    dec = node
    while dec:
        # if pipe or not node:
        out[count] = address_translation[dec % 8]
        dec //= 8
        count += 1

    # if pipe != 0 or node == 0:
    out[0] = address_translation[pipe]
    # else:
    out[1] = address_translation[count - 1]

    return out


def print_pipe_address(addr):
    print(''.join([f"{x:02X}" for x in addr]))


print_pipe_address(pipe_address(0,0))
print_pipe_address(pipe_address(0,1))
print_pipe_address(pipe_address(0,2))
print_pipe_address(pipe_address(0,3))
print_pipe_address(pipe_address(0,4))
print_pipe_address(pipe_address(0,5))

print()
print_pipe_address(pipe_address(1,0))
print_pipe_address(pipe_address(1,1))
print_pipe_address(pipe_address(1,2))
print_pipe_address(pipe_address(1,3))
print_pipe_address(pipe_address(1,4))
print_pipe_address(pipe_address(1,5))

# print_pipe_address(pipe_address(0o11,0))
# print_pipe_address(pipe_address(0o21,0))
# print_pipe_address(pipe_address(0o31,0))
# print_pipe_address(pipe_address(0o41,0))
# print_pipe_address(pipe_address(0o51,0))

# def level_to_address(level):
#     if level:
#         return 1 << ((level - 1) * 3)
#     return 0


# def _setup_address(node_address):
#     node_mask_check = 0xFFFF
#     count = 0
#     while node_address & node_mask_check:
#         node_mask_check <<= 3
#         count += 1
#     print(f"multicast_level {count}")
#     print(f"node mask {(~node_mask_check) & 0xFFFF:X}")
#     node_mask = (~node_mask_check & 0xFFFF)
#     parent_mask = node_mask >> 3
#     parent_node = node_address & parent_mask
#     print(f"parent node {parent_node:o}")
#     i = node_address
#     m = parent_mask
#     while m:
#         i >>= 3
#         m >>= 3
#     print(f"parent_pipe {i}")

# print()
# _setup_address(0)
# _setup_address(1)
# _setup_address(0o11)
