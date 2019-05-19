''' loglink lib '''
import struct
import array


class x25crc(object):
    '''x25 CRC'''
    def __init__(self, buf=None):
        self.crc = 0xffff
        if buf is not None:
            if isinstance(buf, str):
                self.accumulate_str(buf)
            else:
                self.accumulate(buf)

    def accumulate(self, buf):
        '''add in some more bytes'''
        accum = self.crc
        for b in buf:
            tmp = b ^ (accum & 0xff)
            tmp = (tmp ^ (tmp<<4)) & 0xFF
            accum = (accum>>8) ^ (tmp<<8) ^ (tmp<<3) ^ (tmp>>4)
        #self.crc = accum ^ 0xFFFF
        self.crc = accum

    def accumulate_str(self, buf):
        '''add in some more bytes'''
        accum = self.crc
        import array
        bytes = array.array('B')
        bytes.fromstring(buf)
        self.accumulate(bytes)

class Loglink(object):
    '''base Loglink class'''
    def __init__(self):
        self._buf = bytearray()
        self.buf_index = 0
        self.expected_length = 7

    def buf_len(self):
        return len(self._buf) - self.buf_index

    def decode(self, msgbuf):
        try:
            header1, header2, datalen, msgId = struct.unpack('>ccHB', msgbuf[:5])
        except struct.error as emsg:
            #raise Exception('Unable to unpack Loglink header: %s' % emsg)
            print('Unable to unpack Loglink header: %s' % emsg)
            return

        if header1 != 'L' and header2 != 'G':
            #raise Exception("invalid Loglink head '%c %c'" % (header1, header2))
            print("invalid Loglink head '%c %c'" % (header1, header2))
            return

        if datalen != len(msgbuf)-7:
            #raise Exception('invalid Loglink message length. Got %u expected %u, msgId=%u' % (len(msgbuf)-7, datalen, msgId))
            print('invalid Loglink message length. Got %u expected %u, msgId=%u' % (len(msgbuf)-7, datalen, msgId))
            return

        if not msgId in loglink_map:
            #raise Exception('unknown Loglink message ID %d' % msgId)
            print('unknown Loglink message ID %d' % msgId)
            return

        type = loglink_map[msgId]
        fmt = type.format

        try:
            crc, = struct.unpack('>H', msgbuf[-2:])
        except struct.error as emsg:
            #raise Exception('Unable to unpack Loglink CRC: %s' % emsg)
            print('Unable to unpack Loglink CRC: %s' % emsg)
            return
            
        crcbuf = msgbuf[2:-2]
        crc2 = x25crc(crcbuf)
        if crc != crc2.crc:
            #raise Exception('invalid Loglink CRC in msgID %u 0x%04x should be 0x%04x' % (msgId, crc, crc2.crc))
            print('invalid Loglink CRC in msgID %u 0x%04x should be 0x%04x' % (msgId, crc, crc2.crc))
            return

        mbuf = msgbuf[5:-2]
        if len(mbuf) < datalen:
            #raise Exception('Bad message of type %s length %u needs %s' % (type, len(mbuf), datalen))
            print('Bad message of type %s length %u needs %s' % (type, len(mbuf), datalen))
            return

        try:
            t = struct.unpack(fmt, mbuf)
        except struct.error as emsg:
            #raise Exception('Unable to unpack Loglink payload type=%s fmt=%s payloadLength=%u: %s' % (type, fmt, len(mbuf), emsg))
            print('Unable to unpack Loglink payload type=%s fmt=%s payloadLength=%u: %s' % (type, fmt, len(mbuf), emsg))
            return
        
        if msgId != LOGLINK_MSG_ID_SEND_DATA:
            try:
                m = type(*t)
            except Exception as emsg:
                #raise Exception('Unable to instantiate Loglink message of type %s : %s' % (type, emsg))
                print('Unable to instantiate Loglink message of type %s : %s' % (type, emsg))
                return
        else:
            m = loglink_send_data_message(t[0], t[1], t[2:])

        m._header     = "LG"
        m._len        = datalen
        m._msg_id     = msgId
        m._payload    = msgbuf[5:-2]
        m._crc        = crc
        m._msgbuf     = msgbuf

        return m


    def parse_char(self, c):
        '''input some data bytes, possibly returning a new message'''
        self._buf.extend(c)
        if self.buf_len() >= 4:
            sbuf = self._buf[self.buf_index:self.buf_index+4]
            (header1, header2, self.expected_length) = struct.unpack('>ccH', sbuf)
            self.expected_length += 7
        if self.expected_length >= 7 and self.buf_len() >= self.expected_length:
            mbuf = array.array('B', self._buf[self.buf_index:self.buf_index+self.expected_length])
            m = self.decode(mbuf)
            self.buf_index += self.expected_length
            self.expected_length = 4
            return m
        return None



class Loglink_message(object):
    '''base Loglink message class'''
    def __init__(self, msgId):
        self._header     = "LG"
        self._len        = None
        self._msg_id     = msgId
        self._payload    = None
        self._crc        = None
        self._msgbuf     = None

    def pack(self, payload):
        self._len = len(payload)
        self._payload = payload
        self._msgbuf = struct.pack('>ccHB', 'L', 'G', self._len, self._msg_id) + self._payload
        self._crc = x25crc(self._msgbuf[2:]).crc
        self._msgbuf += struct.pack('>H', self._crc)
        return self._msgbuf


LOGLINK_MSG_ID_REQUEST_INFO=0
LOGLINK_MSG_ID_RESPONSE_INFO=1
LOGLINK_MSG_ID_REQUEST_DATA=2
LOGLINK_MSG_ID_SEND_DATA=3
LOGLINK_MSG_ID_REQUEST_END=4


class Loglink_request_info_message(Loglink_message):
        '''The request info message'''
        format = ''

        def __init__(self):
                Loglink_message.__init__(self, LOGLINK_MSG_ID_REQUEST_INFO)

        def pack(self):
                return Loglink_message.pack(self, '')

class Loglink_response_info_message(Loglink_message):
        '''The response info message'''
        format = '>I'

        def __init__(self, size):
                Loglink_message.__init__(self, LOGLINK_MSG_ID_RESPONSE_INFO)
                self._size = size

        def pack(self):
                return Loglink_message.pack(self, struct.pack(format, self._size))

class loglink_request_data_message(Loglink_message):
        '''The request data message'''
        format = '>I'

        def __init__(self, package_num):
                Loglink_message.__init__(self, LOGLINK_MSG_ID_REQUEST_DATA)
                self._package_num = package_num

        def pack(self):
                return Loglink_message.pack(self, struct.pack('>I', self._package_num))


class loglink_send_data_message(Loglink_message):
        '''The send data message'''
        format = '>HH1024B'

        def __init__(self, package_num, length, data):
                Loglink_message.__init__(self, LOGLINK_MSG_ID_SEND_DATA)
                self._package_num = package_num
                self._length = length
                self._data = data

        def pack(self):
                return Loglink_message.pack(self, struct.pack(format, self._package_num. self._length))
                

class loglink_request_end_message(Loglink_message):
        '''The request end message'''
        format = ''

        def __init__(self):
                Loglink_message.__init__(self, LOGLINK_MSG_ID_REQUEST_END)

        def pack(self):
                return Loglink_message.pack(self, '')


                
loglink_map = {
    LOGLINK_MSG_ID_REQUEST_INFO : Loglink_request_info_message,
    LOGLINK_MSG_ID_RESPONSE_INFO : Loglink_response_info_message,
    LOGLINK_MSG_ID_REQUEST_DATA : loglink_request_data_message,
    LOGLINK_MSG_ID_SEND_DATA : loglink_send_data_message,
    LOGLINK_MSG_ID_REQUEST_END : loglink_request_end_message,
}

                