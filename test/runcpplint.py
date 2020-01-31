#!/usr/bin/env python3

# Adapted from https://github.com/code-freak/codeclimate-cpplint.git

import subprocess
import sys
import json
import re
import argparse
import math

ECLIPSE_REGEX = re.compile(r'^(.*):([0-9]+):\s+(.*?)\s+\[([^\]]+)\]\s+\[([0-9]+)\]')

parser = argparse.ArgumentParser()
parser.add_argument('--files', nargs='+')
args = parser.parse_args()


def get_issue(line):
    matches = ECLIPSE_REGEX.match(line)
    print(line)
    if matches is None:
        return
    (file, line, message, check, level) = matches.group(1, 2, 3, 4, 5)
    issue = {
        #"fingerprint": "%x"%(abs(hash(check+message+file+level))),
        "type": "issue",
        "check_name": check,
        "description": message,
        "categories": ["Style"],
        "location": {
            "path": file,
            "lines": {
                "begin": int(line),
                "end": int(line)
            }
        }
    }
    return issue

def run_cpplint(files):
    cmd = ["cpplint", "--linelength=120", "--filter=-legal", "--recursive"]
    cmd.extend(files)
    try:
        output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        print(output)
    except subprocess.CalledProcessError as e:
        output = e.output

    if len(output):
        output = output.decode('utf-8')
        issues = []
        for line in output.splitlines():
            result = get_issue(line)
            if result:
                issues.append(result)

        issues.append({
                "fingerprint": "%x"%(len(issues)),
                "type": "issue",
                "description": "Total number of issues: %d"%len(issues),
                "categories": ["Style"],
                "location": {
                    "path": ".",
                    "lines": {
                        "begin": 1,
                        "end": 1
                    }
                }
            }
        )

        print(">>> Writing JSON report to gl-code-quality-report.json")
        with open('gl-code-quality-report.json', 'w') as f:
            json.dump(issues, f, indent=4)



def main():
    run_cpplint(args.files)
    print("\0")
    pass


if __name__ == "__main__":
    main()
