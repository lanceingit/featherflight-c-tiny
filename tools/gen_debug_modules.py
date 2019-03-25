import sys



#arg1:in_path arg2:out_path

print sys.argv

file_in_path = sys.argv[1].lower()
file_out_name = "debug_module_list.h"
file_out_path = sys.argv[2]+"/"+file_out_name

print file_out_path

file_in = open(file_in_path)
file_out = open(file_out_path,"w+")


str='''#pragma once

char* debug_module_list[DEBUG_ID_MAX] = 
{
    "NULL",\n''' 
file_out.write(str)


while True:
    line = file_in.readline()
    if not line:
        break
    print line
    if line.find("DEBUG_ID_MIN")!= -1:
        print "find line"
        break;
#    else:
#        line = file_in.readline()
        

while True:
    line = file_in.readline()
    if not line:
        break
    print line
    if line.find("DEBUG_ID_MAX")!= -1:
        print "end line"
        break;
    else:
        
        start_pos = line.find("DEBUG_ID_")
        if start_pos >= 0:
            start_pos += len("DEBUG_ID_")
            stop_pos = line.find(",")
            if stop_pos >= 0:
                name = line[start_pos:stop_pos]
                print "name:" + name
                str='''\t"%s",\n''' % name
                file_out.write(str)
    
        #line = file_in.readline()
        
        
        
str='''};\n'''
file_out.write(str)
        
    
file_in.close()
file_out.close()






