#! /usr/bin/env python
# Generate random floating numbers to make a kd-tree.
# Usage: ./kdtree-gen.py [filename] [number of rows] [number of columns]

import sys
import random


if len(sys.argv) == 1:
    row_num = 10
    col_num = 2
else:
    row_num = sys.argv[2]
    col_num = sys.argv[3]


def format_row(ncols=col_num, seed=10):
    row = ""
    for _ in range(int(ncols)):
        col = random.uniform(seed, seed+5)
        row += f"{col}\t"
    row += "\n"

    return row


if __name__ == "__main__":
    with open(f"../{sys.argv[1]}", "w+") as input_file:
        for _ in range(int(row_num)):
            input_file.write(format_row())

