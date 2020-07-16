
#export PYTHONHOME=/usr/local/lib/python3.6

import os
#import felix_config as felix


from qrreader import qrread
import re
import sys
#sys.path.append("/afs/cern.ch/work/r/rowang/public/FELIX/swROD_NTuple/")
#from BoardReadoutTest import board_check_hit


def ReadQRCode():
        
        if(len(sys.argv) == 2):
        
                run_path = sys.argv[1]
		os.system('ping_sca --hostname um-felix2 --from_host_port 12340 --to_host_port 12350 --elinks 00,01,02 > /afs/cern.ch/user/s/stgcic/public/benchtest/tools/SCA_ID_search/output_temp.txt')
		f = open("/afs/cern.ch/user/s/stgcic/public/benchtest/tools/SCA_ID_search/output_temp.txt")
		lines = 0
		SCA = (f.readlines())[-3:]
		scaid_read={}
		for sca in SCA:
		    message = sca.split(" ")
		    if message[0]=="elink":
			lines += 1
			#elink = message[1]
			scaid_read[lines] = re.search(r"\d+",message[-1]).group(0)
			print scaid_read[lines]
			#FEB_dic[elink] = FEB_dic[elink] + "-" + sca_id

		# check all the sca status
		if lines == 3:
		    os.system("echo find all the SCA")
		elif lines == 0:
		    os.system("echo no SCA found")
		else:
		    os.system("echo loss some SCA")
                
                fqrcode = open(run_path+"qrcode.txt","w")
                
                testname = ""
                for i in range(0,3,1):
                        #bdname="bd"+str(i)
                        bdname,scaid=qrread()
                        print bdname
                        if scaid == scaid_read[i+1]:
		            print '\33[32m' + 'scan correct' + '\33[0m'
                        else:
                            print '\33[31m' + 'scan error' + '\33[0m'
                        print bdname

                        testname = testname + scaid + " "
                        fqrcode.write(bdname + "\n")
                fqrcode.close()
                print "testname:"+testname
        
        
if(__name__=="__main__"):
    ReadQRCode()
