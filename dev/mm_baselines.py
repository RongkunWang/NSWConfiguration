import argparse, copy, multiprocessing, os, time

def options():
    # put more options here when the time comes
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-b", action="store_true", help="Measure baselines")
    parser.add_argument("-c",                      help="Config file")
    parser.add_argument("-s",                      help="N(samples)")
    parser.add_argument("-f",                      help="Comma-separated list of FEBs")
    parser.add_argument("-x",                      help="Executable file to run")
    return parser.parse_args()

def main():

    parallel = True
    ops = options()
    if not any([ops.b]):
        fatal("Please use -b")

    # command line args
    if not ops.x:
        fatal("Please give an executable with -x")
    if not os.path.isfile(ops.x):
        fatal("This executable doesnt exist: %s" % (ops.x))
    if not ops.c:
        fatal("Please give an config with -c")
    if not os.path.isfile(ops.c):
        fatal("This config file doesnt exist: %s" % (ops.c))
    if not ops.s:
        fatal("Please give a number of samples to measure with -s")
    if not ops.f:
        fatal("Please give a list of FEBs to measure baselines")

    # announcements
    febs = ops.f.split(",")
    if not febs:
        fatal("I couldnt parse this correctly: %s" % (ops.f))
    print("FEBs to measure baselines:")
    for feb in febs:
        if not feb:
            continue
        print(" - %s" % (feb))

    # dictionary for multiprocessing
    cfg = {}
    cfg["script"] = ops.x
    cfg["config"] = ops.c
    cfg["sample"] = ops.s

    # loops over FEBs
    jobs = []
    for feb in febs:
        if not feb:
            continue
        this_cfg = copy.deepcopy(cfg)
        this_cfg["feb"] = feb
        this_cfg["out"] = "baselines_%s.txt" % (feb)
        if parallel:
            proc = multiprocessing.Process(target=process_baselines, args=(this_cfg,))
            jobs.append(proc)
            proc.start()
        else:
            process_baselines(this_cfg)

def process_baselines(cfg):
    # cmd = "%(script)s -c %(config)s -n MMFE8_%(feb)s -s %(sample)s --baseline --dump | tee %(out)s" % (cfg)
    cmd = "%(script)s -c %(config)s -n MMFE8_%(feb)s -s %(sample)s --baseline --dump" % (cfg)
    os.system(cmd)

def fatal(message):
    import sys
    sys.exit("Error in %s: %s" % (__file__, message))

if __name__ == "__main__":
    main()
