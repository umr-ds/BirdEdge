#!/usr/bin/env python3

import subprocess
import argparse
import sys
import io
import logging
import csv
import time

KEYWORDS = {"RAM": lambda ram: ram[1].split("/")[0],
            "SWAP": lambda swap: swap[1].split("/")[0],
            "IRAM": lambda iram: iram[1].split("/")[0],
            "CPU": lambda cpu: [core.split("%")[0] for core in cpu[1][1:-1].split(",")],
            "EMC_FREQ": lambda s: s[1].split("%")[0],
            "GR3D_FREQ": lambda s: s[1].split("%")[0],
            "POM_5V_IN": lambda s: dict(zip(["CUR", "AVG"], s[1].split("/"))),
            "POM_5V_GPU": lambda s: dict(zip(["CUR", "AVG"], s[1].split("/"))),
            "POM_5V_CPU": lambda s: dict(zip(["CUR", "AVG"], s[1].split("/"))),
            }

parser = argparse.ArgumentParser(
    prog='tegrastats',
    description='A tegrastats wrapper',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
parser.add_argument("-v", "--verbose", help="increase output verbosity", action="count", default=0)
parser.add_argument("-i", "--interval", help="sample the information in <milliseconds>", default=1000)
parser.add_argument("-l", "--logfile", help="dump the output of tegrastats to <filename>", type=argparse.FileType('w'), default=sys.stdout)


def parse_tegraline(line):
    out = {"TIME": time.time()}

    elems = line.split()
    last_keyword_i = -1
    for i in range(len(elems)+1):
        # continue if there is no keyword
        if i < len(elems) and elems[i] not in KEYWORDS:
            continue

        # if there has been a keyword before, parse it
        if last_keyword_i >= 0:
            keyword = elems[last_keyword_i]
            res = KEYWORDS[keyword](elems[last_keyword_i:i])
            if isinstance(res, list):
                res = {f"{keyword}_{i}": x for i, x in enumerate(res)}
            elif isinstance(res, dict):
                res = {f"{keyword}_{k}": v for k, v in res.items()}
            else:
                res = {keyword: res}

            out.update(res)

        last_keyword_i = i

    return out


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    args = parser.parse_args()

    logging_level = logging.WARNING - 10 * args.verbose
    logging.basicConfig(level=logging_level)

    cmd = ["tegrastats", "--interval", str(args.interval)]
    if args.verbose:
        cmd += ["--verbose"]

    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )

    writer = None

    for line in io.TextIOWrapper(process.stdout, encoding="utf-8"):
        parsed = parse_tegraline(line[:-1])

        if not writer:
            writer = csv.DictWriter(args.logfile, fieldnames=parsed.keys())
            writer.writeheader()

        writer.writerow(parsed)
