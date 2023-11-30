import RFCommRAW

response = RFCommRAW.communicate('CA:FE:CA:FE:BA:BE', 10, 1024, b'AAAA\r\n')
