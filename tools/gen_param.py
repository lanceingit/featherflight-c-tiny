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

struct param_val param_list[] = {
'''    
    f.write(str)

    for i in range(len(param)):    
        param_group = param.keys()[i]
        #print param[param_group]
        for param_name in param[param_group]:    
            param_name_low = param_name.lower()
            str='''\t{"%s", param_get_%s, param_set_%s},\n''' %(param_name,param_name_low,param_name_low)
            f.write(str)
        f.write("\n")    

    str='''};\n\n'''
    f.write(str)
    
    for group_name in param:    
        str='''struct %s_param_s* %s_param;\n''' % (group_name.lower(),group_name.lower())
        f.write(str)

    f.close()
    
def generate_api_h(directory, param):
    '''api.h'''
    
    param_len=0;
    
    f = open(os.path.join(directory, "param_api.h"), mode='w')
    
    str='''#pragma once

#include "param.h"

extern struct param_val param_list[];\n''' 
    f.write(str)

        
    # for i in range(len(param)):    
        # param_group = param.keys()[i]
        # str='''\nvoid param_register_%s(void* param);\n''' % param_group.lower()
        # f.write(str)
        # for param_name in param[param_group]:   
            # param_len += 1
            # str='''float param_get_%s(void);\n''' %param_name.lower()
            # f.write(str)
            # str='''void param_set_%s(float v);\n''' %param_name.lower()
            # f.write(str)
   
    for i in range(len(param)):    
        param_group = param.keys()[i]
        param_len += len(param[param_group])
    str='''\n#define PARAM_LIST_MAX %d\n\n''' % param_len
    f.write(str)
    
    for group_name in param:       
        str='''#include "param_gen_%s.h"\n''' %group_name.lower()
        f.write(str)    
    
    f.close()    
 

def generate_param_group_h(directory, group, param):
    '''param_group.h'''
    
    group_low = group.lower()
    
    f = open(os.path.join(directory, "param_gen_"+group_low+".h"), mode='w')
    str = '''#pragma once

#include <stdint.h>
#include "param.h"
#include "param_api.h"\n\n'''
    f.write(str)

    str='''struct %s_param_s {\n''' %group_low 
    f.write(str)

    for param_name in param:  
        str='''\tfloat %s;\n''' %param_name.lower()
        f.write(str)

    str='''};\n\n'''
    f.write(str)

    str='''extern struct %s_param_s* %s_param;\n''' % (group_low,group_low)
    f.write(str)
    
    str='''static inline void param_register_%s(void* param)
{
	%s_param = (struct %s_param_s*)param;
}\n\n''' %(group_low,group_low,group_low)
    f.write(str)


    for param_name in param: 
        param_name_low = param_name.lower()
        str='''static inline float param_get_%s(void)
{
    return %s_param->%s;
}

static inline void param_set_%s(float v)
{
    %s_param->%s = v;
}\n\n''' %(param_name_low,group_low,param_name_low,param_name_low,group_low,param_name_low)
        f.write(str)

    f.close()    
    

def generate_param_add_h(directory, group, param):  
    '''group_param.h'''

    group_low = group.lower()
    
    f = open(os.path.join(directory, group_low+"_param.h"), mode='w')

    str = '''#pragma once
    
#include "param.h" 

static struct _params_local_s {\n'''
    f.write(str)
    
    for param_name in param: 
        str = '''\tfloat _%s;\n''' %param_name
        f.write(str)

    str = '''} _local_param = {\n'''
    f.write(str)

    for param_name in param: 
        str = '''\t._%s = %ff,\n''' %(param_name,param[param_name])
        f.write(str)
        
    str = '''};\n'''
    f.write(str)


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
        line += 1
except:
    param_change=True    


param_change=True    
if param_change==True:  
    for i in files(path+"src/param", "param_gen_*.h"):
        #print i
        os.remove(i)  
      
    generate_api_c(path+"/src/param", param)
    generate_api_h(path+"/src/param", param)
    for i in range(len(param)):    
        param_group = param.keys()[i]
        generate_param_group_h(path+"/src/param", param_group, param[param_group])
        generate_param_add_h(path+"/src/param", param_group, param[param_group])
    print "generate param api success\n"
else:
    print "param no change,skip generate\n"
    