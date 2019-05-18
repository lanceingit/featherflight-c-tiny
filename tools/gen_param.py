import os
import json
import glob
import re
import sys

def files(curr_dir = '.', ext = '*.exe'):
    """curr file"""
    for i in glob.glob(os.path.join(curr_dir, ext)):
        yield i
        
def generate_api_c(directory, param):
    '''api.c'''
    f = open(os.path.join(directory, "param_api.c"), mode='w')
    
    str='''#include <stdint.h>
#include "param.h"
#include "param_api.h"  
 
'''
    f.write(str)


    str = '''struct _params_global_s _global_param = {\n'''
    f.write(str)

    for i in range(len(param)): 
        param_group = param.keys()[i]
        for param_name in param[param_group]: 
            str = '''\t._%s = %ff,\n''' %(param_name,param[param_group][param_name])
            f.write(str)
        
    str = '''};\n'''
    f.write(str)

    
    str='''
struct param_val param_list[] = {
'''    
    f.write(str)

    for i in range(len(param)):    
        param_group = param.keys()[i]
        #print param[param_group]
        for param_name in param[param_group]:    
            str='''\t{"%s", &_global_param._%s},\n''' %(param_name, param_name)
            f.write(str)
        f.write("\n")    

    str='''};\n\n'''
    f.write(str)
    
    f.close()
    
def generate_api_h(directory, param):
    '''api.h'''
    
    param_len=0;
    
    f = open(os.path.join(directory, "param_api.h"), mode='w')
    
    str='''#pragma once

#include "param.h"
'''
    f.write(str)

    str='''
struct _params_global_s {
'''    
    f.write(str)
    for i in range(len(param)): 
        param_group = param.keys()[i]
        for param_name in param[param_group]: 
            str = '''\tfloat _%s;\n''' %param_name
            f.write(str)
    str = '''};\n'''
    f.write(str)

    str='''
extern struct _params_global_s _global_param; 
extern struct param_val param_list[];\n''' 
    f.write(str)
   
    for i in range(len(param)):    
        param_group = param.keys()[i]
        param_len += len(param[param_group])
    str='''\n#define PARAM_LIST_MAX %d\n\n''' % param_len
    f.write(str)
        
    f.close()    
 


path = sys.argv[1]    

j = open(path+"/src/config/ff.param", mode='r')   

param = json.load(j)
#print param

#print len(param)

try:  
    f = open(os.path.join(path+"/src/param", "param_api.c"), mode='r')
    lines = f.readlines()
    line=5
    param_change=False
    for i in range(len(param)):    
        param_group = param.keys()[i]
        for param_name in param[param_group]:    
            if(lines[line].find(param_name) != -1):
                line += 1
            else:    
                param_change=True
                break
except BaseException,e:
    print e
    param_change=True    

#param_change=True    
if param_change==True:        
    generate_api_c(path+"/src/param", param)
    generate_api_h(path+"/src/param", param)
    print "generate param api success\n"
else:
    print "param no change,skip generate\n"
    