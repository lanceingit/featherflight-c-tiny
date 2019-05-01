import serial
import time
import msvcrt
import sys


def erase_chars(N):
    if N == 0: return
    for i in range(0,N):
        sys.stdout.write(" ")            
    for i in range(0,N):
        sys.stdout.write("\b") 

def client_write(data):
    s.write(data)    

def client_read():
    data = s.read(1024)
    return data

try:        
    s = serial.Serial("com33",115200,timeout=0.01)

    command=""
    index=0
    command_history=[]
    history_index = 0

    sys.stdout.write("\n>")

    while True:
        try:
            data = client_read()
        except:
            a=1
        else: 
            numnew = len(data)
            if numnew != 0:
                print(data),
            
        if msvcrt.kbhit():
            ch = msvcrt.getch()
            if ch == "\r":
                if len(command)>0 and (len(command_history) == 0 or command_history[-1] != command):
                    command_history.append(command)        
                    if len(command_history) > 50:
                        del command_history[0]                
                history_index = len(command_history)
                
                sys.stdout.write("\n")
                command += "\n"
                #print(command),
                client_write(command)
                command = ""
                index = 0
            elif ch == '\b':
                if index > 0:
                    sys.stdout.write("\b \b")
                    #print(" \b"),
                    #command = command[:-1]
                    str_list=list(command)
                    str_list.pop(index-1)
                    command="".join(str_list)
                    index -= 1                
                    if index < len(command):
                        for i in range(0,len(command)-index+1):
                            sys.stdout.write(" ")            
                        for i in range(0,len(command)-index+1):
                            sys.stdout.write("\b")            
                        sys.stdout.write(command[index:])
                        for i in range(0,len(command)-index):
                            sys.stdout.write("\b")            
            elif ord(ch) == 0xe0:    
                ch = msvcrt.getch()
                if ord(ch) == 0x48:
                    if history_index > 0:
                        history_index -= 1           
                    
                    sys.stdout.write("\r>")
                    erase_chars(len(command))    
                    if history_index == len(command_history):
                        command = ''
                    else:
                        command = command_history[history_index]
                    sys.stdout.write(command)         
                    index = len(command)                
                    #print("up")
                elif ord(ch) == 0x50:
                    if history_index < len(command_history):
                        history_index += 1
                        
                    sys.stdout.write("\r>")
                    erase_chars(len(command))    
                    
                    if history_index == len(command_history):
                        command = ''
                    else:
                        command = command_history[history_index]
                    sys.stdout.write(command)                    
                    index = len(command)                
                    #print("down")
                elif ord(ch) == 0x4b:
                    sys.stdout.write("\b")
                    index -= 1
                    #print("left")
                elif ord(ch) == 0x4d:
                    #print("right")
                    if index < len(command):
                        sys.stdout.write(command[index])
                        index += 1
                    if index > len(command):
                        index = len(command)
            else:
                sys.stdout.write(ch)
                #command += ch
                #command.insert(index, ch)
                str_list=list(command)
                str_list.insert(index, ch)
                command="".join(str_list)            
                index += 1
                if index < len(command):
                    sys.stdout.write(command[index:])
                    for i in range(0,len(command)-index):
                        sys.stdout.write("\b")
                
        sys.stdout.flush()        
        time.sleep(0.01)

except(KeyboardInterrupt):    
    print "exit..."
    s.close()
    time.sleep(1)
    
        







