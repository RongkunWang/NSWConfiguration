#!/usr/bin/env python

# Script for configuring LUTs on all the trigger processors.
# Takes a directory path.
# Reads all the LUTs from the given directory path. Drives the write_tp_bram executable, with the correct params, per each lookup table found.

import subprocess
import argparse
import os, sys
import distutils.spawn

def build_lut_tree(dirpath):
    """ Build a tree of all LUTs present in dirpath; keys are trigger processor names, and values are lists of
        filepaths of LUTs that should be sent to that specific trigger processor. LUT filenames should follow the
        format {A,C}[01-16]_<lut_name>.mem.
    """
    # files = [f for f in os.listdir(dirpath) if os.path.isfile(f)]
    files = os.listdir(dirpath)
    lut_tree = {}
    for f in files:
        tp_name = ""
        sector = f.split("_")[0]
        if dirpath.endswith("/mm"):
            tp_name = "MMTP_" + sector
        elif dirpath.endswith("/stgc"):
            tp_name = "sTGCTP_" + sector

        if tp_name not in lut_tree:
            lut_tree[tp_name] = [os.path.join(dirpath, f)]
        else:
            lut_tree[tp_name].append(os.path.join(dirpath, f))

    return lut_tree

def enumerate_luts(dirpath):
    """ Enumerates all LUTs that are to be sent to the trigger processors in the given directory,
        returning a map of trigger processor board to the LUTs that are to be sent to them.
    """
    root = [f for f in os.listdir(dirpath) if os.path.isdir(f)]
    if "mm" not in root or "stgc" not in root:
        print("[x] Error: root directory doesn't contain mm or stgc folders. exiting...")
        sys.exit(1)
    mm_tree = build_lut_tree(dirpath + "/mm")
    stgc_tree = build_lut_tree(dirpath + "/stgc")
    # Can be replaced with | operator in Python >3.9.0
    return {**mm_tree, **stgc_tree}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dryRun", help = "Parse all files, but DON'T actually send to TPs", action="store_true", default=False)
    parser.add_argument("--configFile", "-c", required = True, type = str, help="NSWConfiguration database path")
    parser.add_argument("directory", type = str, help="Root directory containing LUT files")
    args = parser.parse_args()

    if not distutils.spawn.find_executable("write_tp_bram"):
        print("[x] Error: could not find write_tp_bram exectuable, make sure your $PATH is set correctly. Exiting...")
        sys.exit(1)

    # Enumerate all luts in the given directory
    lut_tree = enumerate_luts(".")

    for tp in lut_tree:
        print(f"[+] Now configuring {tp}")
        for lut in lut_tree[tp]:
            command = f"write_tp_bram -m {lut} -t {tp} -c ../data/integration_config.json --dryrun"
            print(command)
            try:
                subprocess.check_output(command, shell=True)
            except subprocess.CalledProcessError as ex:
                print(f"[x] Error: write_tp_bram exited with error code {ex.returncode}. Program stdout:")
                print(ex.output)
                print("[x] Exiting...")
                sys.exit(1)

if __name__ == "__main__":
    main()