#!/bin/env python3
import sys

if len(sys.argv) != 3:
    print("[!] Usage: ./parse_config.py [firmware.hex] [config.conf]")
    exit(1)

with open(sys.argv[1], "r") as f:
    hex_lines = f.read().strip().splitlines()

base = 0
mem = {}  # addr -> byte
for line in hex_lines:
    assert line[0] == ':'
    ll = int(line[1:3], 16)
    addr = int(line[3:7], 16)
    rtype = int(line[7:9], 16)
    data = bytes.fromhex(line[9:9+ll*2])
    if rtype == 0x04:  # ELA
        base = int.from_bytes(data, 'big')
    elif rtype == 0x00:  # data
        abs_addr = (base << 16) + addr
        for i, b in enumerate(data):
            mem[abs_addr + i] = b

def w(lo_addr):
    lo = mem.get(lo_addr, 0xFF)
    hi = mem.get(lo_addr+1, 0xFF)
    return (hi << 8) | lo

words = [w(0x300000 + i*2) for i in range(7)]
uids  = [mem.get(0x200000 + i, 0xFF) for i in range(8)]

content = ""
for i, v in enumerate(words):
    content += f"word{i + 1}=0x{v:04X}\n"

for i, v in enumerate(uids):
    content += f"user_id{i}=0x{v:02X}\n"

with open(sys.argv[2], "w") as f:
    f.write(content)

