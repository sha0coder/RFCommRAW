import RFCommRAW


r = RFCommRAW.communicate('ca:fe:ba:be:ca:fe', 2, 1024, b'ATZ\r\n')
print(r)
