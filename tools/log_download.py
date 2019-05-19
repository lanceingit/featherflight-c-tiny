import loglink
import serial
import time
import binascii
import array
import struct
import sys
import os


s = serial.Serial("com33",921600,timeout=0.001)

log_name = 'log_' + time.strftime('%Y-%m-%d_%H-%M-%S',time.localtime(time.time())) + '.log'

print log_name

log = open(log_name, 'wb')


link = loglink.Loglink()

msg = loglink.Loglink_request_info_message()        
packed_data = msg.pack()
s.write(packed_data)
print "request info send:"
for i in range(len(packed_data)):
    print "0x" + binascii.hexlify(packed_data[i]),
print "\n"


log_size = 0
last_num=0
rate=0

t1 = time.time()
t3 = time.time()

last_recv_data = time.time()

while True:
    try:
        data = s.read(8000)
    except:
        a=1
    else:           
        numnew = len(data)
        #print data
        if numnew != 0:
            #print "recv:(%d)" %numnew
            #for i in range(len(data)):
            #    print "0x" + binascii.hexlify(data[i]),
            #print "\n"
        
            msg = link.parse_char(data)
            if msg:
                if msg._msg_id == loglink.LOGLINK_MSG_ID_RESPONSE_INFO:
                    log_size = msg._size
                    print "log size:%fk" %(log_size/1024)
                    package_num = 0                    
                    package_total = log_size/1024
                    if log_size%1024 > 0:
                        package_total += 1
                    msg = loglink.loglink_request_data_message(package_num)        
                    packed_data = msg.pack()
                    s.write(packed_data)
                    print "request data send:"
                    for i in range(len(packed_data)):
                        print "0x" + binascii.hexlify(packed_data[i]),
                    print "\n"
                
                elif msg._msg_id == loglink.LOGLINK_MSG_ID_SEND_DATA:    
                
                    #print "recv data pass: %f" %(time.time() -last_recv_data)
                    last_recv_data = time.time()
                
                    #print msg._payload[4:4+msg._length]
                    #print msg._length                   
                    log.write(msg._payload[4:4+msg._length])
                    package_num += 1
                    #print "package num:%d/%d (%f kb) %dkb/s" %(package_num, package_total, log_size/1024, rate)
                    #print "log size:%f/%f kb" %((package_num*1024),(log_size/1024))
                    if package_num >= package_total:
                        msg = loglink.loglink_request_end_message()        
                        packed_data = msg.pack()
                        s.write(packed_data)
                        print "request end send:"
                        for i in range(len(packed_data)):
                            print "0x" + binascii.hexlify(packed_data[i]),
                        print "\n"                       
     
                        print log_name + " download success!"
                        print "pass:%f" %(time.time()-t3)
                        log.close()
                        raise "end"

                    #else:
                    #    msg = loglink.loglink_request_data_message(package_num)        
                    #    packed_data = msg.pack()
                    #    s.write(packed_data)
                        #print "request data send:"
                        #for i in range(len(packed_data)):
                        #    print "0x" + binascii.hexlify(packed_data[i]),
                        #print "\n"
            #else:
            #    print "[%f]no data" %time.time()
         
    t2 = time.time()
    if t2 - t1 > 1.0:
        rate = package_num-last_num
        last_num  = package_num
        t1 = t2                    
        print "package num:%d/%d (%f kb) %dkb/s" %(package_num, package_total, log_size/1024, rate)
     

    #time.sleep(0.001)




