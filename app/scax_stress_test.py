#!/usr/bin/env python

# This script is for stress testing the SCAX register access interface
# L. Lee 2019


from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os,sys
import csv
import argparse
from collections import OrderedDict
import subprocess
import time
from random import randrange,shuffle
import distutils.spawn

parser = argparse.ArgumentParser()
parser.add_argument("--inputFile","-i",  type = str, help="input csv file", default = "scax_test.txt")
parser.add_argument("--dryRun",    help = "don't run the commands, just print them to screen", action="store_true", default=False)
parser.add_argument("--generate","-g",    help = "generate my own random list of register addresses and values", action="store_true", default=False)
parser.add_argument("--nRegisters","-n", type=int,   help = "number of registers to play with. -1 for all 1024 of them", default=-1)
parser.add_argument("--master","-M", type=int,  help = "which I2c master to test, -1 to loop over all", default=0)
parser.add_argument("--mTests","-m", type=int,  help = "how many times you want to run the test", default=1)
parser.add_argument("--debug","-d",    help = "debug", action="store_true", default=False)

args = parser.parse_args()


def run(cmd):
    proc = subprocess.Popen(cmd,
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE,
    )
    stdout, stderr = proc.communicate()

    return proc.returncode, stdout, stderr


def createRandomRegisterConfig():
	if not args.generate:
		with open(args.inputFile, 'rb') as f:
		    reader = csv.reader(f)
		    listOfRegisters = list(reader)
	else:
		print("... Generating my own list of registers and values")
		listOfRegisters = []
		for regAddr in range(1024):
			listOfRegisters.append(["%04x"%regAddr,"%08x"%randrange(0xffffffff)])
		shuffle(listOfRegisters)

		print("... Writing internally generated register values to generatedRegisterValues.csv")
		with open("generatedRegisterValues.csv",'wb') as resultFile:
			wr = csv.writer(resultFile)
			for item in listOfRegisters:
				wr.writerow(item)

	listOfRegisters = listOfRegisters[:args.nRegisters]

	dictOfRegisters = OrderedDict()
	for (key,value) in listOfRegisters:
		dictOfRegisters[key] = value

	return dictOfRegisters


def main():

	print("\n\n")
	print("Welcome to scax_stress_test. I'm going to write a bunch of values to registers on the SCAX, then read them all back and check for consistency.")

	# Print out the settings
	for setting in dir(args):
		if not setting[0]=="_":
			print( ">>> ... Setting: {: >20} {: >40}".format(setting, eval("args.%s"%setting) ) )


	# Check that I have access to nsw_tp_comm_test
	if not distutils.spawn.find_executable("nsw_tp_comm_test"):
		print("... WARNING: I can't find nsw_tp_comm_test. Make sure it's in your PATH.")
		print("... export PATH=$PATH:/path/to/x86_64-centos7-gcc8-opt/NSWConfiguration/")
		sys.exit(1)


	# Setup of tests

	listOfMasters = [args.master]
	if -1 in listOfMasters:
		listOfMasters = range(16)


	nFailures = 0

	# The actual tests start here

	for iTest in range(args.mTests):

		dictOfRegisters = createRandomRegisterConfig()

		print("... Writing values to registers... (Use -d to see commands and values)")

		for iReg,regAddr in enumerate(dictOfRegisters):

			"""Writing everything"""

			if iReg%10==0:
				print(iReg)

			for master in listOfMasters:
				command = "nsw_tp_comm_test -W -r %s -m %s -s NSW_TrigProc_STGC.I2C_%d.bus%d"%(regAddr,dictOfRegisters[regAddr],master,master)
				if args.debug:
					print(command)

				if not args.dryRun:
					output = subprocess.check_output(command, shell=True)
					output = output.split("\n")
					if args.debug:
						print(output)



		print("... Reading values from registers and checking consistency... (Use -d to see commands and values)")

		for iReg,regAddr in enumerate(dictOfRegisters):

			"""Let's read it back and check against our written values"""

			if iReg%10==0:
				print(iReg)

			for master in listOfMasters:
				command = "timeout 5 nsw_tp_comm_test -R -r %s -s NSW_TrigProc_STGC.I2C_%d.bus%d"%(regAddr,master,master)
				if args.debug:
					print(command)
				if not args.dryRun:
					try:
						output = subprocess.check_output(command, shell=True)
						output = output.split("\n")
						outData = "%02x%02x%02x%02x"%(int(output[5],16),int(output[4],16),int(output[3],16),int(output[2],16) )
						if args.debug:
							print(output)
					except:
						print("... ERROR: Read command not happy")

					before = int(dictOfRegisters[regAddr],16)
					after = int(outData,16)
					print("Register: %s, Read: %x, Expected: %x"%(regAddr,after,before) )
					if before!=after:
						print("... ERROR: Error -- Mismatch!!!")
						nFailures += 1


	print("********** Summary ************")
	print("--> Tested %d registers"%(len(dictOfRegisters)*args.mTests) )
	print("--> ... in %d tests"%(args.mTests) )
	print("--> ... over masters:", listOfMasters )
	print("--> Found %d failures"%(nFailures) )
	print("*******************************")



if __name__ == "__main__":
	main()
