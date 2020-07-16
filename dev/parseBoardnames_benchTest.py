#!/usr/bin/python                                                                                                                                                                                          

import sys,string,random,os,fileinput

def parse_boardnames():

    if(len(sys.argv) == 3):
        
        boardname_txt = sys.argv[1]
        run_path = sys.argv[2]

        input_file = open(boardname_txt, 'r')

        list_lines = input_file.readlines()
        
        
        
        boardnames = []
        
        split_values = []
        

        for line in list_lines:
            
            boardnames.append(line)
            split_values.append(line.split("-"))

        boardname_string = ""
        middlevalue_string = ""
        scaid_string = ""

        for ii in range(len(boardnames)):
            boardname_string = boardname_string+boardnames[ii].strip("\n")
            if(ii != len(boardnames)-1):
                boardname_string = boardname_string+","

        for ii in range(len(boardnames)):
            middlevalue_string = middlevalue_string+split_values[ii][1]
            if(ii != len(boardnames)-1):
                middlevalue_string = middlevalue_string+","
            
        for ii in range(len(boardnames)):
            scaid_string = scaid_string+split_values[ii][2].strip("\n")
            if(ii != len(boardnames)-1):
                scaid_string = scaid_string+"_"

        #print(boardname_string)
        #print(middlevalue_string)
        print(scaid_string)

        return str(scaid_string)

if(__name__=="__main__"):
    parse_boardnames()
