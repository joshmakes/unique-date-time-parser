#!/usr/bin/env python3
import sys
import random
import os

def rand(width):
    return f"{random.randint(0, 10**width - 1):0{width}d}"

def get_datetime_string():
    y  = rand(4)
    mo = rand(2)
    d  = rand(2)
    h  = rand(2)
    mi = rand(2)
    s  = rand(2)

    # Short or longer format datetime string
    if random.choice([True, False]):
        off_h = rand(2)
        off_m = rand(2)
        sign = random.choice(["+", "-"])
        return f"{y}-{mo}-{d}T{h}:{mi}:{s}{sign}{off_h}:{off_m}"
    else:
        return f"{y}-{mo}-{d}T{h}:{mi}:{s}Z"

def main():
    if len(sys.argv) != 3:
        print("Usage: python generate.py <output_file> <num_lines>")
        sys.exit(1)

    out = sys.argv[1]
    num = int(sys.argv[2])

    # Ensure directories exist
    os.makedirs(os.path.dirname(out), exist_ok=True) if os.path.dirname(out) else None

    with open(out, "w") as f:
        for _ in range(num):
            f.write(get_datetime_string() + "\n")

if __name__ == "__main__":
    main()
