import RFCommRAW



channel = 2
bytes_to_read = 1024
timeout = 6
msg = b'ATZ\r\n'

r = RFCommRAW.communicate('ca:fe:ba:be:ca:fe', channel, bytes_to_read, timeout, msg)
print(r)
