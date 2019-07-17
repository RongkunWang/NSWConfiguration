import argparse, copy, multiprocessing, os, time

# Number of VMMs per MMFE8
VMM_NUM = 8

def options():
    # put more options here when the time comes
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-rms",                    help="RMS Factor")
    parser.add_argument("-c",                      help="Config")
    parser.add_argument("-f",                      help="Comma-separated list of FEBs")
    parser.add_argument("-s",			   help="Script to run")
    return parser.parse_args()

def main():

    parallel = True
    ops = options()

    # command line args
    if not ops.rms:
        fatal("Please give an RMS factor with -rms")
    if not ops.c:
        fatal("Please give an config with -c")
    if not ops.s:
        fatal("Please give a script with -s")
    if not os.path.isfile(ops.s):
        fatal("This script file doesnt exist: %s" % (ops.s))
    if not os.path.isfile(ops.c):
        fatal("This config file doesnt exist: %s" % (ops.c))
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
    cfg["rms"] = ops.rms
    cfg["config"] = ops.c
    cfg["script"] = ops.s

    # loops over FEBs
    jobs = []
    for feb in febs:
        if not feb:
            continue
        if parallel:
            proc = multiprocessing.Process(target=process_vmms, args=(cfg,feb))
            jobs.append(proc)
            proc.start()
        else:
            process_vmms(cfg, feb)

def process_vmms(cfg, feb):
    for i in range(VMM_NUM):
       this_cfg = copy.deepcopy(cfg)
       this_cfg["feb"] = feb
       this_cfg["vmm"] = i
       this_cfg["out"] = "thresholds_%s_vmm_%i.txt" % (feb, i)
       process_baselines(this_cfg)
    return

def process_baselines(cfg):
    # cmd = "%(script)s -c %(config)s -n MMFE8_%(feb)s -s %(sample)s --baseline --dump | tee %(out)s" % (cfg)
    # cmd = "%(script)s -c %(config)s -n MMFE8_%(feb)s -V %(vmm)s --rms_factor %(rms)s | tee %(out)s" % (cfg)
    cmd = "%(script)s -c %(config)s -n        %(feb)s -V %(vmm)s --rms_factor %(rms)s | tee %(out)s" % (cfg)
    os.system(cmd)

def fatal(message):
    import sys
    sys.exit("Error in %s: %s" % (__file__, message))

if __name__ == "__main__":
    main()
