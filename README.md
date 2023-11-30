import RFCommRAW

response = RFCommRAW.communicate('CA:FE:BA:BE:CA:FE', 10, 1024, b'AAAA\r\n')



// params:  bssid, channel, recv-size, bytes-to-send


