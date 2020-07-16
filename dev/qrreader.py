

def qrread():
	raw_scan = raw_input("Please scan the board:\n")
	raw_scan.replace(" ","")
        raw_scan.replace("\n","")
        raw_scan.replace("\t","")
        #print "raw_scan : " + raw_scan
        raw_info = raw_scan.split()[0]
        #print "raw_info : " + raw_info
	#board_info0 = 'NNNNNN-N' 
	#board_info1 = 'NNNNN-NNNN'
	split_raw_info = raw_info.split('-')

        #for i in range(0,4,1):
        #	print split_raw_info[i] + "\n"
        
	#board_info0 = split_raw_info[0] + '-' + split_raw_info[1]
	#board_info1 = split_raw_info[2] + '-' + split_raw_info[3]
	#print 'board_info0: ' +  board_info0
	#print 'board_info1: ' + board_info1
	print "Scan finised"
	return raw_info,split_raw_info[2]



